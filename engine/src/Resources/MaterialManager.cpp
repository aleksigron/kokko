#include "Resources/MaterialManager.hpp"

#include <cassert>

#include "rapidjson/document.h"

#include "Core/Array.hpp"
#include "Core/Core.hpp"

#include "Engine/Engine.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/RenderDevice.hpp"
#include "Rendering/Uniform.hpp"

#include "Resources/AssetLoader.hpp"
#include "Resources/TextureManager.hpp"
#include "Resources/ShaderManager.hpp"
#include "Resources/ValueSerialization.hpp"

#include "System/IncludeOpenGL.hpp"

const MaterialId MaterialId::Null = MaterialId{ 0 };

MaterialManager::MaterialManager(
	Allocator* allocator,
	kokko::AssetLoader* assetLoader,
	RenderDevice* renderDevice,
	ShaderManager* shaderManager,
	TextureManager* textureManager) :
	allocator(allocator),
	assetLoader(assetLoader),
	renderDevice(renderDevice),
	shaderManager(shaderManager),
	textureManager(textureManager),
	uniformScratchBuffer(allocator),
	pathHashMap(allocator),
	uidMap(allocator)
{
	data = InstanceData{};
	data.count = 1; // Reserve index 0 as Null instance

	freeListFirst = 0;

	this->Reallocate(80);
}

MaterialManager::~MaterialManager()
{
	for (unsigned int i = 1; i < data.count; ++i)
	{
		data.material[i].uniformData.Release();
	}

	allocator->Deallocate(data.buffer);
}

void MaterialManager::Reallocate(unsigned int required)
{
	if (required <= data.allocated)
		return;

	required = static_cast<unsigned int>(Math::UpperPowerOfTwo(required));

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

	data.material[id.i].uid = kokko::Uid();
	data.material[id.i].transparency = TransparencyType::Opaque;
	data.material[id.i].shaderId = ShaderId{};
	data.material[id.i].cachedShaderDeviceId = 0;
	data.material[id.i].uniformBufferObject = 0;
	data.material[id.i].uniformData = kokko::UniformData(allocator);

	++data.count;

	return id;
}

void MaterialManager::RemoveMaterial(MaterialId id)
{
	data.material[id.i].uniformData.Release();

	// Material isn't the last one
	if (id.i < data.count - 1)
	{
		data.freeList[id.i] = freeListFirst;
		freeListFirst = id.i;
	}

	--data.count;
}

MaterialId MaterialManager::FindMaterialByUid(const kokko::Uid& uid)
{
	KOKKO_PROFILE_FUNCTION();

	auto* pair = uidMap.Lookup(uid);
	if (pair != nullptr)
		return pair->second;

	if (data.count == data.allocated)
		this->Reallocate(data.count + 1);

	Array<uint8_t> file(allocator);

	if (assetLoader->LoadAsset(uid, file))
	{
		MaterialId id = CreateMaterial();

		StringRef fileStr(reinterpret_cast<const char*>(file.GetData()), file.GetCount());

		if (LoadFromConfiguration(id, fileStr))
		{
			data.material[id.i].uid = uid;

			pair = uidMap.Insert(uid);
			pair->second = id;

			return id;
		}
		else
		{
			KK_LOG_ERROR("Material failed to load correctly");

			RemoveMaterial(id);
		}
	}
	else
		KK_LOG_ERROR("AssetLoader couldn't load material asset");

	return MaterialId::Null;
}

MaterialId MaterialManager::FindMaterialByPath(const StringRef& path)
{
	KOKKO_PROFILE_FUNCTION();

	auto uidResult = assetLoader->GetAssetUidByVirtualPath(path);
	if (uidResult.HasValue())
	{
		return FindMaterialByUid(uidResult.GetValue());
	}

	return MaterialId::Null;
}

kokko::Uid MaterialManager::GetMaterialUid(MaterialId id) const
{
	return data.material[id.i].uid;
}

TransparencyType MaterialManager::GetMaterialTransparency(MaterialId id) const
{
	return data.material[id.i].transparency;
}

void MaterialManager::SetMaterialTransparency(MaterialId id, TransparencyType transparency)
{
	data.material[id.i].transparency = transparency;
}

ShaderId MaterialManager::GetMaterialShader(MaterialId id) const
{
	return data.material[id.i].shaderId;
}

void MaterialManager::SetMaterialShader(MaterialId id, ShaderId shaderId)
{
	KOKKO_PROFILE_FUNCTION();

	assert(shaderId != ShaderId::Null);

	MaterialData& material = data.material[id.i];

	const ShaderData& shader = shaderManager->GetShaderData(shaderId);

	material.shaderId = shaderId;
	material.cachedShaderDeviceId = shader.driverId;
	material.transparency = shader.transparencyType;

	material.uniformData.Initialize(shader.uniforms);

	if (material.uniformBufferObject != 0)
		renderDevice->DestroyBuffers(1, &material.uniformBufferObject);

	if (material.uniformData.GetBufferUniforms().GetCount() > 0)
	{
		// Create GPU uniform buffer and allocate storage

		renderDevice->CreateBuffers(1, &material.uniformBufferObject);

		renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, material.uniformBufferObject);

		RenderCommandData::SetBufferStorage bufferStorage{};
		bufferStorage.target = RenderBufferTarget::UniformBuffer;
		bufferStorage.size = material.uniformData.GetUniformBufferSize();
		bufferStorage.data = nullptr;
		bufferStorage.dynamicStorage = true;
		renderDevice->SetBufferStorage(&bufferStorage);
	}
}

static const rapidjson::Value* FindVariableValue(const rapidjson::Value& variablesArray, const StringRef& name)
{
	if (variablesArray.IsArray())
	{
		rapidjson::Value::ConstValueIterator varItr = variablesArray.Begin();
		rapidjson::Value::ConstValueIterator varEnd = variablesArray.End();

		for (; varItr < varEnd; ++varItr)
		{
			rapidjson::Value::ConstMemberIterator nameItr = varItr->FindMember("name");

			if (nameItr != varItr->MemberEnd() &&
				nameItr->value.IsString() &&
				StringRef(nameItr->value.GetString(), nameItr->value.GetStringLength()).ValueEquals(name))
			{
				rapidjson::Value::ConstMemberIterator valueItr = varItr->FindMember("value");
				if (valueItr != varItr->MemberEnd())
					return &valueItr->value;
				else
					return nullptr;
			}
		}
	}

	return nullptr;
}

static unsigned int PrepareUniformArray(const rapidjson::Value* jsonValue, unsigned int uniformArraySize)
{
	unsigned int valueCount = 0;

	if (jsonValue == nullptr)
		KK_LOG_WARN("Failed to read uniform, because JSON value does not exist.");
	else if (jsonValue->IsArray() == false)
		KK_LOG_WARN("Failed to read uniform, because JSON value was not an array.");
	else
	{
		valueCount = jsonValue->Size();

		if (valueCount > uniformArraySize)
		{
			KK_LOG_WARN("JSON array was longer than shader array");
			valueCount = uniformArraySize;
		}
	}

	return valueCount;
}

static void SetBufferUniformValue(
	kokko::UniformData& uniformData,
	const kokko::BufferUniform& uniform,
	const rapidjson::Value* jsonValue,
	Array<unsigned char>& cacheBuffer)
{
	KOKKO_PROFILE_FUNCTION();

	using ValueItr = rapidjson::Value::ConstValueIterator;

	switch (uniform.type)
	{
	case kokko::UniformDataType::Mat4x4:
	{
		Mat4x4f val;

		if (jsonValue != nullptr)
			val = ValueSerialization::Deserialize_Mat4x4f(*jsonValue);

		uniformData.SetValue(uniform, val);
		break;
	}

	case kokko::UniformDataType::Mat4x4Array:
	{
		unsigned int valueCount = PrepareUniformArray(jsonValue, uniform.arraySize);

		cacheBuffer.Resize(valueCount * sizeof(Mat4x4f));
		Mat4x4f* buffer = reinterpret_cast<Mat4x4f*>(cacheBuffer.GetData());

		for (unsigned int i = 0; i < valueCount; ++i)
			buffer[i] = ValueSerialization::Deserialize_Mat4x4f(jsonValue[i]);

		uniformData.SetValueArray(uniform, valueCount, buffer);

		break;
	}

	case kokko::UniformDataType::Mat3x3:
	{
		Mat3x3f val;

		if (jsonValue != nullptr)
			val = ValueSerialization::Deserialize_Mat3x3f(*jsonValue);

		uniformData.SetValue(uniform, val);
		break;
	}

	case kokko::UniformDataType::Mat3x3Array:
	{
		unsigned int valueCount = PrepareUniformArray(jsonValue, uniform.arraySize);

		cacheBuffer.Resize(valueCount * sizeof(Mat3x3f));
		Mat3x3f* buffer = reinterpret_cast<Mat3x3f*>(cacheBuffer.GetData());

		for (unsigned int i = 0; i < valueCount; ++i)
			buffer[i] = ValueSerialization::Deserialize_Mat3x3f(jsonValue[i]);

		uniformData.SetValueArray(uniform, valueCount, buffer);

		break;
	}

	case kokko::UniformDataType::Vec4:
	{
		Vec4f val;

		if (jsonValue != nullptr)
			val = ValueSerialization::Deserialize_Vec4f(*jsonValue);

		uniformData.SetValue(uniform, val);
		break;
	}

	case kokko::UniformDataType::Vec4Array:
	{
		unsigned int valueCount = PrepareUniformArray(jsonValue, uniform.arraySize);

		cacheBuffer.Resize(valueCount * sizeof(Vec4f));
		Vec4f* buffer = reinterpret_cast<Vec4f*>(cacheBuffer.GetData());

		for (unsigned int i = 0; i < valueCount; ++i)
			buffer[i] = ValueSerialization::Deserialize_Vec4f(jsonValue[i]);

		uniformData.SetValueArray(uniform, valueCount, buffer);

		break;
	}

	case kokko::UniformDataType::Vec3:
	{
		Vec3f val;

		if (jsonValue != nullptr)
			val = ValueSerialization::Deserialize_Vec3f(*jsonValue);

		uniformData.SetValue(uniform, val);
		break;
	}

	case kokko::UniformDataType::Vec3Array:
	{
		unsigned int valueCount = PrepareUniformArray(jsonValue, uniform.arraySize);

		cacheBuffer.Resize(valueCount * sizeof(Vec3f));
		Vec3f* buffer = reinterpret_cast<Vec3f*>(cacheBuffer.GetData());

		for (unsigned int i = 0; i < valueCount; ++i)
			buffer[i] = ValueSerialization::Deserialize_Vec3f(jsonValue[i]);

		uniformData.SetValueArray(uniform, valueCount, buffer);

		break;
	}

	case kokko::UniformDataType::Vec2:
	{
		Vec2f val;

		if (jsonValue != nullptr)
			val = ValueSerialization::Deserialize_Vec2f(*jsonValue);

		uniformData.SetValue(uniform, val);
		break;
	}

	case kokko::UniformDataType::Vec2Array:
	{
		unsigned int valueCount = PrepareUniformArray(jsonValue, uniform.arraySize);

		cacheBuffer.Resize(valueCount * sizeof(Vec2f));
		Vec2f* buffer = reinterpret_cast<Vec2f*>(cacheBuffer.GetData());

		for (unsigned int i = 0; i < valueCount; ++i)
			buffer[i] = ValueSerialization::Deserialize_Vec2f(jsonValue[i]);

		uniformData.SetValueArray(uniform, valueCount, buffer);

		break;
	}

	case kokko::UniformDataType::Float:
	{
		float val = 0.0f;

		if (jsonValue != nullptr)
			val = ValueSerialization::Deserialize_Float(*jsonValue);

		uniformData.SetValue(uniform, val);
		break;
	}

	case kokko::UniformDataType::FloatArray:
	{
		unsigned int valueCount = PrepareUniformArray(jsonValue, uniform.arraySize);

		cacheBuffer.Resize(valueCount * sizeof(float));
		float* buffer = reinterpret_cast<float*>(cacheBuffer.GetData());

		for (unsigned int i = 0; i < valueCount; ++i)
			buffer[i] = ValueSerialization::Deserialize_Float(jsonValue[i]);

		uniformData.SetValueArray(uniform, valueCount, buffer);

		break;
	}

	case kokko::UniformDataType::Int:
	{
		int val = 0;

		if (jsonValue != nullptr)
			val = ValueSerialization::Deserialize_Int(*jsonValue);

		uniformData.SetValue(uniform, val);
		break;
	}

	case kokko::UniformDataType::IntArray:
	{
		unsigned int valueCount = PrepareUniformArray(jsonValue, uniform.arraySize);

		cacheBuffer.Resize(valueCount * sizeof(int));
		int* buffer = reinterpret_cast<int*>(cacheBuffer.GetData());

		for (unsigned int i = 0; i < valueCount; ++i)
			buffer[i] = ValueSerialization::Deserialize_Int(jsonValue[i]);

		uniformData.SetValueArray(uniform, valueCount, buffer);

		break;
	}

	default:
		break;
	}
}

bool MaterialManager::LoadFromConfiguration(MaterialId id, StringRef config)
{
	KOKKO_PROFILE_FUNCTION();

	using ValueItr = rapidjson::Value::ConstValueIterator;
	using MemberItr = rapidjson::Value::ConstMemberIterator;

	rapidjson::Document doc;
	doc.Parse(config.str, config.len);

	MemberItr shaderItr = doc.FindMember("shader");
	if (shaderItr == doc.MemberEnd() || shaderItr->value.IsString() == false)
		return false;

	StringRef path(shaderItr->value.GetString(), shaderItr->value.GetStringLength());
	ShaderId shaderId = shaderManager->GetIdByPath(path);

	if (shaderId == ShaderId::Null)
		return false;

	// This initializes material uniforms from the shader's data
	SetMaterialShader(id, shaderId);

	MaterialData& material = data.material[id.i];
	const ShaderData& shader = shaderManager->GetShaderData(shaderId);

	MemberItr variablesItr = doc.FindMember("variables");
	const rapidjson::Value* varValue = nullptr;
	bool variablesArrayIsValid = variablesItr != doc.MemberEnd() && variablesItr->value.IsArray();

	for (auto& uniform : material.uniformData.GetBufferUniforms())
	{
		if (variablesArrayIsValid)
			varValue = FindVariableValue(variablesItr->value, uniform.name);
		else
			varValue = nullptr;

		SetBufferUniformValue(material.uniformData, uniform, varValue, uniformScratchBuffer);
	}

	// TEXTURE UNIFORMS

	for (auto& uniform : material.uniformData.GetTextureUniforms())
	{
		TextureId textureId = TextureId{ 0 };

		if (variablesArrayIsValid &&
			(varValue = FindVariableValue(variablesItr->value, uniform.name)) != nullptr &&
			varValue->IsString())
		{
			StringRef path(varValue->GetString(), varValue->GetStringLength());
			textureId = textureManager->GetIdByPath(path);
		}

		if (textureId == TextureId::Null)
		{
			// TODO: Find a more robust solution to find default values for textures
			if (uniform.name.StartsWith(StringRef("normal")))
				textureId = textureManager->GetId_EmptyNormal();
			else
				textureId = textureManager->GetId_White2D();
		}

		uniform.textureId = textureId;

		const TextureData& texture = textureManager->GetTextureData(textureId);
		assert(texture.textureObjectId != 0);
		uniform.textureObject = texture.textureObjectId;
	}

	UpdateUniformsToGPU(id);

	return true;
}

void MaterialManager::UpdateUniformsToGPU(MaterialId id)
{
	KOKKO_PROFILE_FUNCTION();
	
	const MaterialData& material = data.material[id.i];
	const kokko::UniformData& uniforms = material.uniformData;

	unsigned int uniformBufferSize = uniforms.GetUniformBufferSize();

	// Update uniform buffer object on the GPU
	if (uniformBufferSize > 0)
	{
		static const size_t stackBufferSize = 2048;
		unsigned char stackBuffer[stackBufferSize];
		unsigned char* uniformBuffer = nullptr;

		if (uniformBufferSize <= stackBufferSize)
			uniformBuffer = stackBuffer;
		else
			uniformBuffer = static_cast<unsigned char*>(allocator->Allocate(uniformBufferSize));

		uniforms.WriteToUniformBuffer(uniformBuffer);

		renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, material.uniformBufferObject);
		renderDevice->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, uniformBufferSize, uniformBuffer);

		if (uniformBuffer != stackBuffer)
			allocator->Deallocate(uniformBuffer);
	}
}
