#include "Resources/MaterialManager.hpp"

#include <cassert>

#include "rapidjson/document.h"

#include "Core/String.hpp"
#include "Core/Hash.hpp"

#include "Engine/Engine.hpp"

#include "Memory/Allocator.hpp"

#include "Resources/ResourceManager.hpp"
#include "Resources/Texture.hpp"
#include "Resources/ValueSerialization.hpp"

#include "System/File.hpp"

MaterialManager::MaterialManager(Allocator* allocator) :
	allocator(allocator),
	nameHashMap(allocator)
{
	data = InstanceData{};
	data.count = 1; // Reserve index 0 as Null instance

	freeListFirst = 0;

	this->Reallocate(32);
}

MaterialManager::~MaterialManager()
{
	for (unsigned int i = 1; i < data.count; ++i)
		if (data.material[i].uniformData != nullptr)
			allocator->Deallocate(data.material[i].uniformData);

	allocator->Deallocate(data.buffer);
}

void MaterialManager::Reallocate(unsigned int required)
{
	if (required <= data.allocated)
		return;

	required = Math::UpperPowerOfTwo(required);

	InstanceData newData;
	newData.buffer = allocator->Allocate((sizeof(unsigned int) + sizeof(MaterialData)) * required);
	newData.count = data.count;
	newData.allocated = required;

	newData.freeList = static_cast<unsigned int*>(newData.buffer);
	newData.material = reinterpret_cast<MaterialData*>(newData.freeList + required);

	if (data.buffer != nullptr)
	{
		// Since the whole freelist needs to be copied, combine copies of freeList and material
		// Aligment of MaterialData is 8 bytes, allocated needs to be an even number
		size_t copyBytes = data.allocated * sizeof(unsigned int) + data.count * sizeof(MaterialData);
		std::memcpy(newData.buffer, data.buffer, copyBytes);

		allocator->Deallocate(data.buffer);
	}

	data = newData;
}

MaterialId MaterialManager::CreateMaterial()
{
	MaterialId id;

	if (freeListFirst == 0)
	{
		if (data.count == data.allocated)
			this->Reallocate(data.count + 1);

		// If there are no freelist entries, first <objectCount> indices must be in use
		id.i = data.count;
	}
	else
	{
		id.i = freeListFirst;
		freeListFirst = data.freeList[freeListFirst];
	}

	data.material[id.i].uniformCount = 0;
	data.material[id.i].uniformDataSize = 0;
	data.material[id.i].uniformData = nullptr;

	++data.count;

	return id;
}

void MaterialManager::RemoveMaterial(MaterialId id)
{
	if (data.material[id.i].uniformData != nullptr)
	{
		allocator->Deallocate(data.material[id.i].uniformData);
		data.material[id.i].uniformData = nullptr;
	}

	// Material isn't the last one
	if (id.i < data.count - 1)
	{
		data.freeList[id.i] = freeListFirst;
		freeListFirst = id.i;
	}

	--data.count;
}

MaterialId MaterialManager::GetIdByPath(StringRef path)
{
	uint32_t hash = Hash::FNV1a_32(path.str, path.len);

	HashMap<uint32_t, MaterialId>::KeyValuePair* pair = nameHashMap.Lookup(hash);
	if (pair != nullptr)
		return pair->second;

	if (data.count == data.allocated)
		this->Reallocate(data.count + 1);

	Buffer<char> file(allocator);
	String pathStr(allocator, path);

	if (File::ReadText(pathStr.GetCStr(), file))
	{
		MaterialId id = CreateMaterial();

		if (LoadFromConfiguration(id, file.Data()))
		{
			pair = nameHashMap.Insert(hash);
			pair->second = id;

			return id;
		}
		else
		{
			RemoveMaterial(id);
		}
	}

	return MaterialId{};
}

void MaterialManager::SetShader(MaterialId id, const Shader* shader)
{
	const unsigned int idx = id.i;
	MaterialData& material = data.material[idx];

	if (material.uniformData != nullptr) // Release old uniform data
	{
		allocator->Deallocate(material.uniformData);
		material.uniformData = nullptr;
	}

	material.shader = shader->nameHash;
	material.transparency = shader->transparencyType;

	// Copy uniform information
	material.uniformCount = shader->materialUniformCount;

	unsigned int usedData = 0;

	for (unsigned i = 0, count = shader->materialUniformCount; i < count; ++i)
	{
		const ShaderUniform* mu = shader->materialUniforms + i;
		MaterialUniform& u = material.uniforms[i];

		u.location = mu->location;
		u.nameHash = mu->nameHash;
		u.type = mu->type;
		u.dataOffset = usedData;

		// Increment the amount of data the uniforms have used
		unsigned int typeIndex = static_cast<unsigned int>(mu->type);
		usedData += ShaderUniform::TypeSizes[typeIndex];
	}

	material.uniformDataSize = usedData;

	if (usedData > 0)
	{
		material.uniformData = static_cast<unsigned char*>(allocator->Allocate(usedData));

		// TODO: Set default values for uniforms
	}
	else
		material.uniformData = nullptr;
}

void SetUniformI(MaterialData& material, unsigned int uniformIndex, int value)
{
	if (material.uniformData != nullptr && material.uniformCount > uniformIndex)
	{
		unsigned char* uData = material.uniformData + material.uniforms[uniformIndex].dataOffset;
		int* uniform = reinterpret_cast<int*>(uData);
		*uniform = value;
	}
}

void SetUniformF(MaterialData& material, unsigned int uniformIndex, float value)
{
	if (material.uniformData != nullptr && material.uniformCount > uniformIndex)
	{
		unsigned char* uData = material.uniformData + material.uniforms[uniformIndex].dataOffset;
		float* uniform = reinterpret_cast<float*>(uData);
		*uniform = value;
	}
}

void SetUniformFv(MaterialData& material, unsigned int uniformIndex, const float* values, unsigned int count)
{
	if (material.uniformData != nullptr && material.uniformCount > uniformIndex)
	{
		unsigned char* uData = material.uniformData + material.uniforms[uniformIndex].dataOffset;
		std::memcpy(uData, values, sizeof(float) * count);
	}
}

void SetUniformTex2D(MaterialData& material, unsigned int uidx, unsigned int value)
{
	assert(material.uniforms[uidx].type == ShaderUniformType::Tex2D);
	SetUniformI(material, uidx, value);
}

void SetUniformTexCube(MaterialData& material, unsigned int uidx, unsigned int value)
{
	assert(material.uniforms[uidx].type == ShaderUniformType::TexCube);
	SetUniformI(material, uidx, value);
}

void SetUniformMat4x4(MaterialData& material, unsigned int uidx, const Mat4x4f& value)
{
	assert(material.uniforms[uidx].type == ShaderUniformType::Mat4x4);
	SetUniformFv(material, uidx, value.ValuePointer(), 16);
}

void SetUniformVec4(MaterialData& material, unsigned int uidx, const Vec4f& value)
{
	assert(material.uniforms[uidx].type == ShaderUniformType::Vec4);
	SetUniformFv(material, uidx, value.ValuePointer(), 4);
}

void SetUniformVec3(MaterialData& material, unsigned int uidx, const Vec3f& value)
{
	assert(material.uniforms[uidx].type == ShaderUniformType::Vec3);
	SetUniformFv(material, uidx, value.ValuePointer(), 3);
}

void SetUniformVec2(MaterialData& material, unsigned int uidx, const Vec2f& value)
{
	assert(material.uniforms[uidx].type == ShaderUniformType::Vec2);
	SetUniformFv(material, uidx, value.ValuePointer(), 2);
}

void SetUniformFloat(MaterialData& material, unsigned int uidx, float value)
{
	assert(material.uniforms[uidx].type == ShaderUniformType::Float);
	SetUniformF(material, uidx, value);
}

void SetUniformInt(MaterialData& material, unsigned int uidx, unsigned int value)
{
	assert(mu.uniforms[uidx].type == ShaderUniformType::Int);
	SetUniformI(material, uidx, value);
}

bool MaterialManager::LoadFromConfiguration(MaterialId id, char* config)
{
	using ValueItr = rapidjson::Value::ConstValueIterator;
	using MemberItr = rapidjson::Value::ConstMemberIterator;

	ResourceManager* res = Engine::GetInstance()->GetResourceManager();

	rapidjson::Document doc;
	doc.ParseInsitu(config);

	MemberItr shaderItr = doc.FindMember("shader");
	if (shaderItr == doc.MemberEnd() || shaderItr->value.IsString() == false)
		return nullptr;

	StringRef path(shaderItr->value.GetString(), shaderItr->value.GetStringLength());
	Shader* shader = res->GetShader(shaderItr->value.GetString());

	if (shader == nullptr)
		return false;

	// This initializes material uniforms from the shader's data
	SetShader(id, shader);

	MemberItr variablesItr = doc.FindMember("variables");

	if (variablesItr != doc.MemberEnd() && variablesItr->value.IsArray())
	{
		ValueItr varItr = variablesItr->value.Begin();
		ValueItr varEnd = variablesItr->value.End();

		for (; varItr < varEnd; ++varItr)
		{
			if (varItr->IsObject() == false)
				continue;

			MemberItr nameItr = varItr->FindMember("name");
			MemberItr valueItr = varItr->FindMember("value");

			if (nameItr == varItr->MemberEnd() || !nameItr->value.IsString() ||
				valueItr == varItr->MemberEnd())
				continue;

			const char* nameStr = nameItr->value.GetString();
			unsigned int nameLen = nameItr->value.GetStringLength();

			int varIndex = -1;
			uint32_t varNameHash = Hash::FNV1a_32(nameStr, nameLen);

			const unsigned int uniformCount = data.material[id.i].uniformCount;

			// Find the index at which there's a variable with the same name
			for (unsigned int i = 0; i < uniformCount; ++i)
			{
				if (shader->materialUniforms[i].nameHash == varNameHash)
				{
					varIndex = i;
					break;
				}
			}

			// The variable was found
			if (varIndex >= 0)
			{
				// Now let's try to read the value

				MaterialData& material = data.material[id.i];
				ShaderUniformType type = material.uniforms[varIndex].type;

				const rapidjson::Value& varVal = valueItr->value;

				using namespace ValueSerialization;

				switch (type)
				{
					case ShaderUniformType::Mat4x4:
						SetUniformMat4x4(material, varIndex, Deserialize_Mat4x4f(varVal));
						break;

					case ShaderUniformType::Vec4:
						SetUniformVec4(material, varIndex, Deserialize_Vec4f(varVal));
						break;

					case ShaderUniformType::Vec3:
						SetUniformVec3(material, varIndex, Deserialize_Vec3f(varVal));
						break;

					case ShaderUniformType::Vec2:
						SetUniformVec2(material, varIndex, Deserialize_Vec2f(varVal));
						break;

					case ShaderUniformType::Float:
						SetUniformFloat(material, varIndex, Deserialize_Float(varVal));
						break;

					case ShaderUniformType::Int:
						SetUniformInt(material, varIndex, Deserialize_Int(varVal));
						break;

					case ShaderUniformType::Tex2D:
					{
						const char* texturePath = varVal.GetString();
						Texture* texture = res->GetTexture(texturePath);
						if (texture != nullptr)
							SetUniformTex2D(material, varIndex, texture->nameHash);
					}
						break;

					case ShaderUniformType::TexCube:
					{
						const char* texturePath = varVal.GetString();
						Texture* texture = res->GetTexture(texturePath);
						if (texture != nullptr)
							SetUniformTexCube(material, varIndex, texture->nameHash);
					}
						break;
				}
			}
		}
	}

	return true;
}
