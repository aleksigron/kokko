#include "Resources/MaterialManager.hpp"

#include <cassert>

#include "rapidjson/document.h"

#include "Core/String.hpp"
#include "Core/Hash.hpp"

#include "Engine/Engine.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/RenderDevice.hpp"

#include "Resources/ResourceManager.hpp"
#include "Resources/ShaderManager.hpp"
#include "Resources/Texture.hpp"
#include "Resources/ValueSerialization.hpp"

#include "System/File.hpp"
#include "System/IncludeOpenGL.hpp"

MaterialManager::MaterialManager(
	Allocator* allocator,
	RenderDevice* renderDevice,
	ShaderManager* shaderManager,
	ResourceManager* resourceManager) :
	allocator(allocator),
	renderDevice(renderDevice),
	shaderManager(shaderManager),
	resourceManager(resourceManager),
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

	data.material[id.i].shaderId = ShaderId{};
	data.material[id.i].cachedShaderDeviceId = 0;
	data.material[id.i].textureCount = 0;
	data.material[id.i].uniformBufferObject = 0;
	data.material[id.i].uniformBufferSize = 0;
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

void MaterialManager::SetShader(MaterialId id, ShaderId shaderId)
{
	const unsigned int idx = id.i;
	MaterialData& material = data.material[idx];

	if (material.uniformData != nullptr) // Release old uniform data
	{
		allocator->Deallocate(material.uniformData);
		material.uniformData = nullptr;
	}

	const ShaderData& shader = shaderManager->GetShaderData(shaderId);

	material.shaderId = shaderId;
	material.cachedShaderDeviceId = shader.driverId;
	material.transparency = shader.transparencyType;

	// Copy uniform information

	material.textureCount = shader.textureUniformCount;
	material.uniformDataSize = shader.uniformDataSize;
	material.uniformBufferSize = shader.uniformBufferSize;
	material.uniformCount = shader.bufferUniformCount;

	for (unsigned int i = 0, count = shader.bufferUniformCount; i < count; ++i)
		material.bufferUniforms[i] = shader.bufferUniforms[i];

	for (unsigned int i = 0, count = shader.textureUniformCount; i < count; ++i)
		material.textureUniforms[i] = shader.textureUniforms[i];

	// Allocate CPU side data buffer
	if (material.uniformDataSize > 0)
		material.uniformData = static_cast<unsigned char*>(allocator->Allocate(material.uniformDataSize));
	else
		material.uniformData = nullptr;

	// Create GPU uniform buffer and allocate storage
	if (material.uniformBufferSize > 0)
	{
		if (material.uniformBufferObject == 0)
			renderDevice->CreateBuffers(1, &material.uniformBufferObject);

		renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, material.uniformBufferObject);
		renderDevice->SetBufferData(RenderBufferTarget::UniformBuffer, material.uniformBufferSize, nullptr, RenderBufferUsage::StaticDraw);
	}
	else if (material.uniformBufferObject != 0)
	{
		renderDevice->DestroyBuffers(1, &material.uniformBufferObject);
		material.uniformBufferObject = 0;
	}
}

bool MaterialManager::LoadFromConfiguration(MaterialId id, char* config)
{
	using ValueItr = rapidjson::Value::ConstValueIterator;
	using MemberItr = rapidjson::Value::ConstMemberIterator;

	rapidjson::Document doc;
	doc.ParseInsitu(config);

	MemberItr shaderItr = doc.FindMember("shader");
	if (shaderItr == doc.MemberEnd() || shaderItr->value.IsString() == false)
		return false;

	StringRef path(shaderItr->value.GetString(), shaderItr->value.GetStringLength());
	ShaderId shaderId = shaderManager->GetIdByPath(path);

	if (shaderId.IsNull())
		return false;

	// This initializes material uniforms from the shader's data
	SetShader(id, shaderId);

	const ShaderData& shader = shaderManager->GetShaderData(shaderId);

	MemberItr variablesItr = doc.FindMember("variables");

	if (variablesItr != doc.MemberEnd() && variablesItr->value.IsArray())
	{
		ValueItr varItr = variablesItr->value.Begin();
		ValueItr varEnd = variablesItr->value.End();

		MaterialData& material = data.material[id.i];

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
			bool isTexture = false;
			uint32_t varNameHash = Hash::FNV1a_32(nameStr, nameLen);

			// Find the index at which there's a variable with the same name
			for (unsigned int i = 0, count = shader.bufferUniformCount; i < count; ++i)
			{
				if (shader.bufferUniforms[i].nameHash == varNameHash)
				{
					varIndex = i;
					break;
				}
			}

			if (varIndex < 0)
			{
				for (unsigned int i = 0, count = shader.textureUniformCount; i < count; ++i)
				{
					if (shader.textureUniforms[i].nameHash == varNameHash)
					{
						isTexture = true;
						varIndex = i;
						break;
					}
				}
			}

			// The variable was found
			if (varIndex >= 0)
			{
				if (isTexture)
				{
					Texture* texture = resourceManager->GetTexture(valueItr->value.GetString());

					if (texture != nullptr)
						material.textureUniforms[varIndex].textureName = texture->driverId;
				}
				else
				{
					const BufferUniform& uniform = material.bufferUniforms[varIndex];
					const rapidjson::Value& varVal = valueItr->value;

					switch (uniform.type)
					{
					case UniformDataType::Mat4x4:
					{
						Mat4x4f val = ValueSerialization::Deserialize_Mat4x4f(varVal);
						uniform.SetValueFloatVec(material.uniformData, val.ValuePointer(), 16);
						break;
					}

					case UniformDataType::Vec4:
					{
						Vec4f val = ValueSerialization::Deserialize_Vec4f(varVal);
						uniform.SetValueFloatVec(material.uniformData, val.ValuePointer(), 4);
						break;
					}

					case UniformDataType::Vec3:
					{
						Vec3f val = ValueSerialization::Deserialize_Vec3f(varVal);
						uniform.SetValueFloatVec(material.uniformData, val.ValuePointer(), 3);
						break;
					}

					case UniformDataType::Vec2:
					{
						Vec2f val = ValueSerialization::Deserialize_Vec2f(varVal);
						uniform.SetValueFloatVec(material.uniformData, val.ValuePointer(), 2);
						break;
					}

					case UniformDataType::Float:
					{
						float val = ValueSerialization::Deserialize_Float(varVal);
						uniform.SetValueFloat(material.uniformData, val);
						break;
					}

					case UniformDataType::Int:
					{
						int val = ValueSerialization::Deserialize_Int(varVal);
						uniform.SetValueInt(material.uniformData, val);
						break;
					}

					default:
						break;
					}
				}
			}
		}

		// Update uniform buffer object on the GPU
		if (material.uniformCount > 0)
		{
			static const size_t stackBufferSize = 2048;
			unsigned char stackBuffer[stackBufferSize];
			unsigned char* uniformBuffer = nullptr;

			if (material.uniformBufferSize <= stackBufferSize)
				uniformBuffer = stackBuffer;
			else
				uniformBuffer = static_cast<unsigned char*>(allocator->Allocate(material.uniformBufferSize));

			for (unsigned int i = 0, count = material.uniformCount; i < count; ++i)
				material.bufferUniforms[i].UpdateToUniformBuffer(material.uniformData, uniformBuffer);

			renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, material.uniformBufferObject);
			renderDevice->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, material.uniformBufferSize, uniformBuffer);

			if (uniformBuffer != stackBuffer)
				allocator->Deallocate(uniformBuffer);
		}
	}

	return true;
}
