#include "Resources/MaterialManager.hpp"

#include <cassert>

#include "rapidjson/document.h"

#include "Core/Array.hpp"
#include "Core/Core.hpp"
#include "Core/String.hpp"
#include "Core/Hash.hpp"

#include "Engine/Engine.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/RenderDevice.hpp"

#include "Resources/TextureManager.hpp"
#include "Resources/ShaderManager.hpp"
#include "Resources/ValueSerialization.hpp"

#include "System/File.hpp"
#include "System/IncludeOpenGL.hpp"

const MaterialId MaterialId::Null = MaterialId{ 0 };

MaterialManager::MaterialManager(
	Allocator* allocator,
	RenderDevice* renderDevice,
	ShaderManager* shaderManager,
	TextureManager* textureManager) :
	allocator(allocator),
	renderDevice(renderDevice),
	shaderManager(shaderManager),
	textureManager(textureManager),
	uniformScratchBuffer(allocator),
	pathHashMap(allocator)
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
		if (data.material[i].buffer != nullptr)
			allocator->Deallocate(data.material[i].buffer);

		if (data.material[i].materialPath != nullptr)
			allocator->Deallocate(data.material[i].materialPath);
	}

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

	data.material[id.i].transparency = TransparencyType::Opaque;
	data.material[id.i].shaderId = ShaderId{};
	data.material[id.i].cachedShaderDeviceId = 0;
	data.material[id.i].uniformBufferObject = 0;
	data.material[id.i].uniforms = UniformList{};
	data.material[id.i].buffer = nullptr;
	data.material[id.i].uniformData = nullptr;
	data.material[id.i].materialPath = nullptr;

	++data.count;

	return id;
}

MaterialId MaterialManager::CreateCopy(MaterialId copyFrom)
{
	KOKKO_PROFILE_FUNCTION();

	const MaterialData& origMaterial = data.material[copyFrom.i];

	MaterialId id = CreateMaterial();
	SetShader(id, origMaterial.shaderId);
	MaterialData& newMaterial = data.material[id.i];

	// Copy buffer uniform data
	std::memcpy(newMaterial.uniformData, origMaterial.uniformData, origMaterial.uniforms.uniformDataSize);

	const TextureUniform* origTextures = origMaterial.uniforms.textureUniforms;
	TextureUniform* newTextures = newMaterial.uniforms.textureUniforms;

	// Copy texture object names
	for (unsigned int uniformIdx = 0; uniformIdx < origMaterial.uniforms.textureUniformCount; ++uniformIdx)
		newTextures[uniformIdx].textureObject = origTextures[uniformIdx].textureObject;

	return id;
}

void MaterialManager::RemoveMaterial(MaterialId id)
{
	if (data.material[id.i].buffer != nullptr)
	{
		allocator->Deallocate(data.material[id.i].buffer);
		data.material[id.i].buffer = nullptr;
	}

	if (data.material[id.i].materialPath != nullptr)
	{
		allocator->Deallocate(data.material[id.i].materialPath);
		data.material[id.i].materialPath = nullptr;
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
	KOKKO_PROFILE_FUNCTION();

	uint32_t hash = Hash::FNV1a_32(path.str, path.len);

	HashMap<uint32_t, MaterialId>::KeyValuePair* pair = pathHashMap.Lookup(hash);
	if (pair != nullptr)
		return pair->second;

	if (data.count == data.allocated)
		this->Reallocate(data.count + 1);

	Array<char> file(allocator);
	String pathStr(allocator, path);

	if (File::ReadText(pathStr.GetCStr(), file))
	{
		MaterialId id = CreateMaterial();

		if (LoadFromConfiguration(id, file.GetData()))
		{
			pair = pathHashMap.Insert(hash);
			pair->second = id;

			// Copy path string
			data.material[id.i].materialPath = static_cast<char*>(allocator->Allocate(path.len + 1));
			std::memcpy(data.material[id.i].materialPath, path.str, path.len);
			data.material[id.i].materialPath[path.len] = '\0';

			return id;
		}
		else
		{
			RemoveMaterial(id);
		}
	}

	return MaterialId::Null;
}

MaterialId MaterialManager::GetIdByPathHash(uint32_t pathHash)
{
	auto pair = pathHashMap.Lookup(pathHash);
	return pair != nullptr ? pair->second : MaterialId{};
}

const MaterialData& MaterialManager::GetMaterialData(MaterialId id) const
{
	assert(id != MaterialId::Null);
	return data.material[id.i];
}

MaterialData& MaterialManager::GetMaterialData(MaterialId id)
{
	assert(id != MaterialId::Null);
	return data.material[id.i];
}

void MaterialManager::SetShader(MaterialId id, ShaderId shaderId)
{
	KOKKO_PROFILE_FUNCTION();

	MaterialData& material = data.material[id.i];

	if (material.buffer != nullptr) // Release old uniform data
	{
		allocator->Deallocate(material.buffer);
		material.buffer = nullptr;
	}

	const ShaderData& shader = shaderManager->GetShaderData(shaderId);

	material.shaderId = shaderId;
	material.cachedShaderDeviceId = shader.driverId;
	material.transparency = shader.transparencyType;

	// Copy uniform information

	material.uniforms.uniformDataSize = shader.uniforms.uniformDataSize;
	material.uniforms.uniformBufferSize = shader.uniforms.uniformBufferSize;
	material.uniforms.bufferUniformCount = shader.uniforms.bufferUniformCount;
	material.uniforms.textureUniformCount = shader.uniforms.textureUniformCount;

	unsigned int uboSize = material.uniforms.uniformDataSize;
	
	if (material.uniforms.bufferUniformCount > 0 ||
		material.uniforms.textureUniformCount > 0)
	{
		size_t bufferSize = material.uniforms.bufferUniformCount * sizeof(BufferUniform) +
			material.uniforms.textureUniformCount * sizeof(TextureUniform) +
			material.uniforms.uniformDataSize;

		material.buffer = allocator->Allocate(bufferSize);

		BufferUniform* bufferBuf = static_cast<BufferUniform*>(material.buffer);
		TextureUniform* textureBuf = reinterpret_cast<TextureUniform*>(bufferBuf + material.uniforms.bufferUniformCount);
		unsigned char* dataBuf = reinterpret_cast<unsigned char*>(textureBuf + material.uniforms.textureUniformCount);

		material.uniforms.bufferUniforms = material.uniforms.bufferUniformCount > 0 ? bufferBuf : nullptr;
		material.uniforms.textureUniforms = material.uniforms.textureUniformCount > 0 ? textureBuf : nullptr;
		material.uniformData = material.uniforms.bufferUniformCount > 0 ? dataBuf : nullptr;

		for (unsigned int i = 0, count = shader.uniforms.bufferUniformCount; i < count; ++i)
			material.uniforms.bufferUniforms[i] = shader.uniforms.bufferUniforms[i];

		for (unsigned int i = 0, count = shader.uniforms.textureUniformCount; i < count; ++i)
			material.uniforms.textureUniforms[i] = shader.uniforms.textureUniforms[i];
	}
	else
	{
		material.buffer = nullptr;
		material.uniforms.bufferUniforms = nullptr;
		material.uniforms.textureUniforms = nullptr;
		material.uniformData = nullptr;
	}

	if (material.uniformBufferObject != 0)
		renderDevice->DestroyBuffers(1, &material.uniformBufferObject);

	if (material.uniforms.bufferUniformCount > 0)
	{
		// Create GPU uniform buffer and allocate storage

		renderDevice->CreateBuffers(1, &material.uniformBufferObject);

		renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, material.uniformBufferObject);

		RenderCommandData::SetBufferStorage bufferStorage{};
		bufferStorage.target = RenderBufferTarget::UniformBuffer;
		bufferStorage.size = material.uniforms.uniformBufferSize;
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

		if (valueCount < uniformArraySize)
			KK_LOG_WARN("JSON didn't provide enough values to fill array");
		else if (valueCount > uniformArraySize)
		{
			KK_LOG_WARN("JSON array was longer than shader array");
			valueCount = uniformArraySize;
		}
	}

	return valueCount;
}

static void SetBufferUniformValue(
	const BufferUniform& uniform,
	unsigned char* uniformData,
	const rapidjson::Value* jsonValue,
	Array<unsigned char>& cacheBuffer)
{
	KOKKO_PROFILE_FUNCTION();

	using ValueItr = rapidjson::Value::ConstValueIterator;

	switch (uniform.type)
	{
	case UniformDataType::Mat4x4:
	{
		Mat4x4f val;

		if (jsonValue != nullptr)
			val = ValueSerialization::Deserialize_Mat4x4f(*jsonValue);

		uniform.SetValueMat4x4f(uniformData, val);
		break;
	}

	case UniformDataType::Mat4x4Array:
	{
		cacheBuffer.Resize(uniform.arraySize * sizeof(Mat4x4f));
		Mat4x4f* buffer = reinterpret_cast<Mat4x4f*>(cacheBuffer.GetData());

		unsigned int valueCount = PrepareUniformArray(jsonValue, uniform.arraySize);
			
		unsigned int i = 0;
		for (; i < valueCount; ++i)
			buffer[i] = ValueSerialization::Deserialize_Mat4x4f(jsonValue[i]);
		for (; i < uniform.arraySize; ++i)
			buffer[i] = Mat4x4f();

		uniform.SetArrayMat4x4f(uniformData, buffer, uniform.arraySize);

		break;
	}

	case UniformDataType::Mat3x3:
	{
		Mat3x3f val;

		if (jsonValue != nullptr)
			val = ValueSerialization::Deserialize_Mat3x3f(*jsonValue);

		uniform.SetValueMat3x3f(uniformData, val);
		break;
	}

	case UniformDataType::Mat3x3Array:
	{
		cacheBuffer.Resize(uniform.arraySize * sizeof(Mat3x3f));
		Mat3x3f* buffer = reinterpret_cast<Mat3x3f*>(cacheBuffer.GetData());

		unsigned int valueCount = PrepareUniformArray(jsonValue, uniform.arraySize);

		unsigned int i = 0;
		for (; i < valueCount; ++i)
			buffer[i] = ValueSerialization::Deserialize_Mat3x3f(jsonValue[i]);
		for (; i < uniform.arraySize; ++i)
			buffer[i] = Mat3x3f();

		uniform.SetArrayMat3x3f(uniformData, buffer, uniform.arraySize);

		break;
	}

	case UniformDataType::Vec4:
	{
		Vec4f val;

		if (jsonValue != nullptr)
			val = ValueSerialization::Deserialize_Vec4f(*jsonValue);

		uniform.SetValueVec4f(uniformData, val);
		break;
	}

	case UniformDataType::Vec4Array:
	{
		cacheBuffer.Resize(uniform.arraySize * sizeof(Vec4f));
		Vec4f* buffer = reinterpret_cast<Vec4f*>(cacheBuffer.GetData());

		unsigned int valueCount = PrepareUniformArray(jsonValue, uniform.arraySize);

		unsigned int i = 0;
		for (; i < valueCount; ++i)
			buffer[i] = ValueSerialization::Deserialize_Vec4f(jsonValue[i]);
		for (; i < uniform.arraySize; ++i)
			buffer[i] = Vec4f();

		uniform.SetArrayVec4f(uniformData, buffer, uniform.arraySize);

		break;
	}

	case UniformDataType::Vec3:
	{
		Vec3f val;

		if (jsonValue != nullptr)
			val = ValueSerialization::Deserialize_Vec3f(*jsonValue);

		uniform.SetValueVec3f(uniformData, val);
		break;
	}

	case UniformDataType::Vec3Array:
	{
		cacheBuffer.Resize(uniform.arraySize * sizeof(Vec3f));
		Vec3f* buffer = reinterpret_cast<Vec3f*>(cacheBuffer.GetData());

		unsigned int valueCount = PrepareUniformArray(jsonValue, uniform.arraySize);

		unsigned int i = 0;
		for (; i < valueCount; ++i)
			buffer[i] = ValueSerialization::Deserialize_Vec3f(jsonValue[i]);
		for (; i < uniform.arraySize; ++i)
			buffer[i] = Vec3f();

		uniform.SetArrayVec3f(uniformData, buffer, uniform.arraySize);

		break;
	}

	case UniformDataType::Vec2:
	{
		Vec2f val;

		if (jsonValue != nullptr)
			val = ValueSerialization::Deserialize_Vec2f(*jsonValue);

		uniform.SetValueVec2f(uniformData, val);
		break;
	}

	case UniformDataType::Vec2Array:
	{
		cacheBuffer.Resize(uniform.arraySize * sizeof(Vec2f));
		Vec2f* buffer = reinterpret_cast<Vec2f*>(cacheBuffer.GetData());

		unsigned int valueCount = PrepareUniformArray(jsonValue, uniform.arraySize);

		unsigned int i = 0;
		for (; i < valueCount; ++i)
			buffer[i] = ValueSerialization::Deserialize_Vec2f(jsonValue[i]);
		for (; i < uniform.arraySize; ++i)
			buffer[i] = Vec2f();

		uniform.SetArrayVec2f(uniformData, buffer, uniform.arraySize);

		break;
	}

	case UniformDataType::Float:
	{
		float val = 0.0f;

		if (jsonValue != nullptr)
			val = ValueSerialization::Deserialize_Float(*jsonValue);

		uniform.SetValueFloat(uniformData, val);
		break;
	}

	case UniformDataType::FloatArray:
	{
		cacheBuffer.Resize(uniform.arraySize * sizeof(float));
		float* buffer = reinterpret_cast<float*>(cacheBuffer.GetData());

		unsigned int valueCount = PrepareUniformArray(jsonValue, uniform.arraySize);

		unsigned int i = 0;
		for (; i < valueCount; ++i)
			buffer[i] = ValueSerialization::Deserialize_Float(jsonValue[i]);
		for (; i < uniform.arraySize; ++i)
			buffer[i] = 0.0f;

		uniform.SetArrayFloat(uniformData, buffer, uniform.arraySize);

		break;
	}

	case UniformDataType::Int:
	{
		int val = 0;

		if (jsonValue != nullptr)
			val = ValueSerialization::Deserialize_Int(*jsonValue);

		uniform.SetValueInt(uniformData, val);
		break;
	}

	case UniformDataType::IntArray:
	{
		cacheBuffer.Resize(uniform.arraySize * sizeof(int));
		int* buffer = reinterpret_cast<int*>(cacheBuffer.GetData());

		unsigned int valueCount = PrepareUniformArray(jsonValue, uniform.arraySize);

		unsigned int i = 0;
		for (; i < valueCount; ++i)
			buffer[i] = ValueSerialization::Deserialize_Int(jsonValue[i]);
		for (; i < uniform.arraySize; ++i)
			buffer[i] = 0;

		uniform.SetArrayInt(uniformData, buffer, uniform.arraySize);

		break;
	}

	default:
		break;
	}
}

bool MaterialManager::LoadFromConfiguration(MaterialId id, char* config)
{
	KOKKO_PROFILE_FUNCTION();

	using ValueItr = rapidjson::Value::ConstValueIterator;
	using MemberItr = rapidjson::Value::ConstMemberIterator;

	rapidjson::Document doc;
	doc.ParseInsitu(config);

	MemberItr shaderItr = doc.FindMember("shader");
	if (shaderItr == doc.MemberEnd() || shaderItr->value.IsString() == false)
		return false;

	StringRef path(shaderItr->value.GetString(), shaderItr->value.GetStringLength());
	ShaderId shaderId = shaderManager->GetIdByPath(path);

	if (shaderId == ShaderId::Null)
		return false;

	// This initializes material uniforms from the shader's data
	SetShader(id, shaderId);

	MaterialData& material = data.material[id.i];
	const ShaderData& shader = shaderManager->GetShaderData(shaderId);

	MemberItr variablesItr = doc.FindMember("variables");
	const rapidjson::Value* varValue = nullptr;
	bool variablesArrayIsValid = variablesItr != doc.MemberEnd() && variablesItr->value.IsArray();

	for (unsigned int uniformIdx = 0; uniformIdx < shader.uniforms.bufferUniformCount; ++uniformIdx)
	{
		const BufferUniform& shaderUniform = shader.uniforms.bufferUniforms[uniformIdx];
		const BufferUniform& materialUniform = material.uniforms.bufferUniforms[uniformIdx];
		
		if (variablesArrayIsValid)
			varValue = FindVariableValue(variablesItr->value, shaderUniform.name);
		else
			varValue = nullptr;

		SetBufferUniformValue(materialUniform, material.uniformData, varValue, uniformScratchBuffer);
	}

	// TEXTURE UNIFORMS

	for (unsigned int uniformIdx = 0; uniformIdx < shader.uniforms.textureUniformCount; ++uniformIdx)
	{
		const TextureUniform& shaderUniform = shader.uniforms.textureUniforms[uniformIdx];
		TextureUniform& materialUniform = material.uniforms.textureUniforms[uniformIdx];

		TextureId textureId = TextureId{ 0 };

		if (variablesArrayIsValid &&
			(varValue = FindVariableValue(variablesItr->value, shaderUniform.name)) != nullptr &&
			varValue->IsString())
		{
			StringRef path(varValue->GetString(), varValue->GetStringLength());
			textureId = textureManager->GetIdByPath(path);
		}

		if (textureId == TextureId::Null)
		{
			// TODO: Find a more robust solution to find default values for textures
			if (shaderUniform.name.StartsWith(StringRef("normal")))
				textureId = textureManager->GetId_EmptyNormal();
			else
				textureId = textureManager->GetId_White2D();
		}

		materialUniform.textureId = textureId;

		const TextureData& texture = textureManager->GetTextureData(textureId);
		assert(texture.textureObjectId != 0);
		materialUniform.textureObject = texture.textureObjectId;
	}

	UpdateUniformsToGPU(id);

	return true;
}

void MaterialManager::UpdateUniformsToGPU(MaterialId id)
{
	KOKKO_PROFILE_FUNCTION();

	MaterialData& material = data.material[id.i];

	// Update uniform buffer object on the GPU
	if (material.uniforms.bufferUniformCount > 0)
	{
		static const size_t stackBufferSize = 2048;
		unsigned char stackBuffer[stackBufferSize];
		unsigned char* uniformBuffer = nullptr;

		if (material.uniforms.uniformBufferSize <= stackBufferSize)
			uniformBuffer = stackBuffer;
		else
			uniformBuffer = static_cast<unsigned char*>(allocator->Allocate(material.uniforms.uniformBufferSize));

		for (unsigned int i = 0, count = material.uniforms.bufferUniformCount; i < count; ++i)
			material.uniforms.bufferUniforms[i].UpdateToUniformBuffer(material.uniformData, uniformBuffer);

		renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, material.uniformBufferObject);
		renderDevice->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, material.uniforms.uniformBufferSize, uniformBuffer);

		if (uniformBuffer != stackBuffer)
			allocator->Deallocate(uniformBuffer);
	}
}
