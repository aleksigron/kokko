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
#include "Resources/MaterialSerializer.hpp"
#include "Resources/ShaderManager.hpp"

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
		kokko::MaterialSerializer serializer(allocator, this, shaderManager, textureManager);

		if (serializer.DeserializeMaterial(id, fileStr))
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
