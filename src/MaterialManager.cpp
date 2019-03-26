#include "MaterialManager.hpp"

#include <cassert>

#include "rapidjson/document.h"

#include "Hash.hpp"
#include "File.hpp"
#include "ValueSerialization.hpp"

#include "Engine.hpp"
#include "ResourceManager.hpp"
#include "Texture.hpp"

MaterialManager::MaterialManager()
{
	data = InstanceData{};
	data.count = 1; // Reserve index 0 as Null instance

	freeListFirst = 0;

	this->Reallocate(32);
}

MaterialManager::~MaterialManager()
{
}

void MaterialManager::Reallocate(unsigned int required)
{
	if (required <= data.allocated)
		return;

	required = Math::UpperPowerOfTwo(required);

	unsigned int objectBytes = 2 * sizeof(unsigned int) + sizeof(MaterialUniformData) + sizeof(TransparencyType);

	InstanceData newData;
	newData.buffer = operator new[](required * objectBytes);
	newData.count = data.count;
	newData.allocated = required;

	newData.freeList = static_cast<unsigned int*>(newData.buffer);
	newData.shader = newData.freeList + required;
	newData.uniform = reinterpret_cast<MaterialUniformData*>(newData.shader + required);
	newData.transparency = reinterpret_cast<TransparencyType*>(newData.uniform + required);

	if (data.buffer != nullptr)
	{
		std::memcpy(newData.freeList, data.freeList, data.allocated * sizeof(unsigned int));
		std::memcpy(newData.shader, data.shader, data.count * sizeof(unsigned int));
		std::memcpy(newData.uniform, data.uniform, data.count * sizeof(MaterialUniformData));
		std::memcpy(newData.transparency, data.transparency, data.count * sizeof(TransparencyType));

		operator delete[](data.buffer);
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

	data.uniform[id.i].count = 0;
	data.uniform[id.i].usedData = 0;
	data.uniform[id.i].data = nullptr;

	++data.count;

	return id;
}

void MaterialManager::RemoveMaterial(MaterialId id)
{
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
		return pair->value;

	if (data.count == data.allocated)
		this->Reallocate(data.count + 1);

	Buffer<char> file = File::ReadText(path);

	if (file.IsValid())
	{
		MaterialId id = CreateMaterial();

		if (LoadFromConfiguration(id, file.Data()))
		{
			pair = nameHashMap.Insert(hash);
			pair->value = id;

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

	if (data.uniform[idx].data != nullptr) // Release old uniform data
	{
		delete[] data.uniform[idx].data;
		data.uniform[idx].data = nullptr;
	}

	data.shader[idx] = shader->nameHash;
	data.transparency[idx] = shader->transparencyType;

	// Copy uniform information
	data.uniform[idx].count = shader->materialUniformCount;

	unsigned int usedData = 0;

	for (unsigned i = 0, count = shader->materialUniformCount; i < count; ++i)
	{
		const ShaderUniform* mu = shader->materialUniforms + i;
		MaterialUniform& u = data.uniform[idx].uniforms[i];

		u.location = mu->location;
		u.nameHash = mu->nameHash;
		u.type = mu->type;
		u.dataOffset = usedData;

		// Increment the amount of data the uniforms have used
		unsigned int typeIndex = static_cast<unsigned int>(mu->type);
		usedData += ShaderUniform::TypeSizes[typeIndex];
	}

	data.uniform[idx].usedData = usedData;

	if (usedData > 0)
	{
		data.uniform[idx].data = new unsigned char[usedData];

		// TODO: Set default values for uniforms
	}
	else
		data.uniform[idx].data = nullptr;
}

void SetUniformI(MaterialUniformData& mu, unsigned int uniformIndex, int value)
{
	unsigned char* uData = mu.data + mu.uniforms[uniformIndex].dataOffset;
	int* uniform = reinterpret_cast<int*>(uData);
	*uniform = value;
}

void SetUniformF(MaterialUniformData& mu, unsigned int uniformIndex, float value)
{
	unsigned char* uData = mu.data + mu.uniforms[uniformIndex].dataOffset;
	float* uniform = reinterpret_cast<float*>(uData);
	*uniform = value;
}

void SetUniformFv(MaterialUniformData& mu, unsigned int uniformIndex, const float* values, unsigned int count)
{
	unsigned char* uData = mu.data + mu.uniforms[uniformIndex].dataOffset;
	float* uniform = reinterpret_cast<float*>(uData);
	std::memcpy(uniform, values, sizeof(float) * count);
}

void SetUniformTex2D(MaterialUniformData& mu, unsigned int uidx, unsigned int value)
{
	assert(mu.uniforms[uidx].type == ShaderUniformType::Tex2D);
	SetUniformI(mu, uidx, value);
}

void SetUniformTexCube(MaterialUniformData& mu, unsigned int uidx, unsigned int value)
{
	assert(mu.uniforms[uidx].type == ShaderUniformType::TexCube);
	SetUniformI(mu, uidx, value);
}

void SetUniformMat4x4(MaterialUniformData& mu, unsigned int uidx, const Mat4x4f& value)
{
	assert(mu.uniforms[uidx].type == ShaderUniformType::Mat4x4);
	SetUniformFv(mu, uidx, value.ValuePointer(), 16);
}

void SetUniformVec4(MaterialUniformData& mu, unsigned int uidx, const Vec4f& value)
{
	assert(mu.uniforms[uidx].type == ShaderUniformType::Vec4);
	SetUniformFv(mu, uidx, value.ValuePointer(), 4);
}

void SetUniformVec3(MaterialUniformData& mu, unsigned int uidx, const Vec3f& value)
{
	assert(mu.uniforms[uidx].type == ShaderUniformType::Vec3);
	SetUniformFv(mu, uidx, value.ValuePointer(), 3);
}

void SetUniformVec2(MaterialUniformData& mu, unsigned int uidx, const Vec2f& value)
{
	assert(mu.uniforms[uidx].type == ShaderUniformType::Vec2);
	SetUniformFv(mu, uidx, value.ValuePointer(), 2);
}

void SetUniformFloat(MaterialUniformData& mu, unsigned int uidx, float value)
{
	assert(mu.uniforms[uidx].type == ShaderUniformType::Float);
	SetUniformF(mu, uidx, value);
}

void SetUniformInt(MaterialUniformData& mu, unsigned int uidx, unsigned int value)
{
	assert(mu.uniforms[uidx].type == ShaderUniformType::Int);
	SetUniformI(mu, uidx, value);
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

			const unsigned int uniformCount = data.uniform[id.i].count;

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

				MaterialUniformData& mu = data.uniform[id.i];
				ShaderUniformType type = mu.uniforms[varIndex].type;

				const rapidjson::Value& varVal = valueItr->value;

				using namespace ValueSerialization;

				switch (type)
				{

					case ShaderUniformType::Mat4x4:
						SetUniformMat4x4(mu, varIndex, Deserialize_Mat4x4f(varVal));
						break;

					case ShaderUniformType::Vec4:
						SetUniformVec4(mu, varIndex, Deserialize_Vec4f(varVal));
						break;

					case ShaderUniformType::Vec3:
						SetUniformVec3(mu, varIndex, Deserialize_Vec3f(varVal));
						break;

					case ShaderUniformType::Vec2:
						SetUniformVec2(mu, varIndex, Deserialize_Vec2f(varVal));
						break;

					case ShaderUniformType::Float:
						SetUniformFloat(mu, varIndex, Deserialize_Float(varVal));
						break;

					case ShaderUniformType::Int:
						SetUniformInt(mu, varIndex, Deserialize_Int(varVal));
						break;

					case ShaderUniformType::Tex2D:
					{
						const char* texturePath = varVal.GetString();
						Texture* texture = res->GetTexture(texturePath);
						if (texture != nullptr)
							SetUniformTex2D(mu, varIndex, texture->nameHash);
					}
						break;

					case ShaderUniformType::TexCube:
					{
						const char* texturePath = varVal.GetString();
						Texture* texture = res->GetTexture(texturePath);
						if (texture != nullptr)
							SetUniformTexCube(mu, varIndex, texture->nameHash);
					}
						break;
				}
			}
		}
	}

	return true;
}
