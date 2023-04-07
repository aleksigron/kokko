#include "Resources/ShaderManager.hpp"

#include <cassert>

#include "rapidjson/document.h"

#include "Core/Core.hpp"
#include "Core/Hash.hpp"
#include "Core/String.hpp"

#include "Memory/Allocator.hpp"

#include "Resources/AssetLoader.hpp"
#include "Resources/ShaderLoader.hpp"

const ShaderId ShaderId::Null = ShaderId{ 0 };

ShaderManager::ShaderManager(
	Allocator* allocator,
	kokko::Filesystem* filesystem,
	kokko::AssetLoader* assetLoader,
	RenderDevice* renderDevice) :
	allocator(allocator),
	filesystem(filesystem),
	assetLoader(assetLoader),
	renderDevice(renderDevice),
	freeListFirst(0),
	uidMap(allocator)
{
	data = InstanceData{};
	data.count = 1; // Reserve index 0 as Null instance

	this->Reallocate(32);
}

ShaderManager::~ShaderManager()
{
	for (unsigned int i = 1; i < data.allocated; ++i)
		allocator->Deallocate(data.shader[i].buffer);

	allocator->Deallocate(data.buffer);
}

void ShaderManager::Reallocate(size_t required)
{
	KOKKO_PROFILE_FUNCTION();

	if (required <= data.allocated)
		return;

	required = Math::UpperPowerOfTwo(required);

	size_t bytes = (sizeof(unsigned int) + sizeof(ShaderData)) * required;

	InstanceData newData;
	newData.buffer = allocator->Allocate(bytes, "ShaderManager.data.buffer");
	newData.count = data.count;
	newData.allocated = required;

	newData.freeList = static_cast<unsigned int*>(newData.buffer);
	newData.shader = reinterpret_cast<ShaderData*>(newData.freeList + required);

	if (data.buffer != nullptr)
	{
		// Aligment of MaterialData is 8 bytes, allocated needs to be an even number

		std::memcpy(newData.freeList, data.freeList, data.allocated * sizeof(unsigned int));
		std::memset(newData.freeList + data.allocated, 0, (newData.allocated - data.allocated) * sizeof(unsigned int));
		std::memcpy(newData.shader, data.shader, data.count * sizeof(ShaderData));
		std::memset(newData.shader + data.count, 0, (newData.allocated - data.count) * sizeof(ShaderData));

		allocator->Deallocate(data.buffer);
	}
	else
	{
		std::memset(newData.buffer, 0, bytes);
	}

	data = newData;
}

ShaderId ShaderManager::CreateShader()
{
	ShaderId id;

	if (freeListFirst == 0)
	{
		if (data.count == data.allocated)
			this->Reallocate(data.count + 1);

		// If there are no freelist entries, first <objectCount> indices must be in use
		id.i = static_cast<unsigned int>(data.count);
	}
	else
	{
		id.i = freeListFirst;
		freeListFirst = data.freeList[freeListFirst];
	}

	data.shader[id.i].buffer = nullptr;
	data.shader[id.i].uniformBlockDefinition = kokko::ConstStringView();
	data.shader[id.i].transparencyType = TransparencyType::Opaque;
	data.shader[id.i].driverId = kokko::RenderShaderId();
	data.shader[id.i].uniforms = kokko::UniformList();

	++data.count;

	return id;
}

void ShaderManager::RemoveShader(ShaderId id)
{
	assert(id != ShaderId::Null);

	auto mapPair = uidMap.Lookup(data.shader[id.i].uid);
	if (mapPair != nullptr)
		uidMap.Remove(mapPair);

	if (data.shader[id.i].buffer != nullptr)
	{
		allocator->Deallocate(data.shader[id.i].buffer);
		data.shader[id.i].buffer = nullptr;
	}
	 
	// If material isn't the last one, add it to the freelist
	if (id.i < data.count - 1)
	{
		data.freeList[id.i] = freeListFirst;
		freeListFirst = id.i;
	}

	--data.count;
}

ShaderId ShaderManager::FindShaderByUid(const kokko::Uid& uid)
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
		auto virtualPathResult = assetLoader->GetAssetVirtualPath(uid);
		const kokko::String& virtualPath = virtualPathResult.GetValue();

		ShaderId id = CreateShader();
		ShaderData& shader = data.shader[id.i];

		kokko::ConstStringView fileString(reinterpret_cast<char*>(file.GetData()), file.GetCount());
		kokko::ShaderLoader loader(allocator, filesystem, renderDevice);

		if (loader.LoadFromFile(shader, virtualPath.GetRef(), fileString, virtualPath.GetRef()))
		{
			data.shader[id.i].uid = uid;

			pair = uidMap.Insert(uid);
			pair->second = id;

			return id;
		}
		else
		{
			KK_LOG_ERROR("Material failed to load correctly");

			RemoveShader(id);
		}
	}
	else
		KK_LOG_ERROR("AssetLoader couldn't load shader asset");

	return ShaderId::Null;
}

ShaderId ShaderManager::FindShaderByPath(kokko::ConstStringView path)
{
	KOKKO_PROFILE_FUNCTION();

	auto uidResult = assetLoader->GetAssetUidByVirtualPath(path);
	if (uidResult.HasValue())
	{
		return FindShaderByUid(uidResult.GetValue());
	}

	return ShaderId::Null;
}
