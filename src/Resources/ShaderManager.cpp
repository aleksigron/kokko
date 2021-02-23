#include "Resources/ShaderManager.hpp"

#include <cassert>

#include "rapidjson/document.h"

#include "Core/String.hpp"
#include "Core/Hash.hpp"

#include "Memory/Allocator.hpp"

#include "Resources/ShaderLoader.hpp"
#include "Resources/ValueSerialization.hpp"

#include "System/File.hpp"

ShaderManager::ShaderManager(Allocator* allocator, RenderDevice* renderDevice) :
	allocator(allocator),
	renderDevice(renderDevice),
	freeListFirst(0),
	nameHashMap(allocator)
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

void ShaderManager::Reallocate(unsigned int required)
{
	if (required <= data.allocated)
		return;

	required = Math::UpperPowerOfTwo(required);

	size_t bytes = (sizeof(unsigned int) + sizeof(ShaderData)) * required;

	InstanceData newData;
	newData.buffer = allocator->Allocate(bytes);
	newData.count = data.count;
	newData.allocated = required;

	newData.freeList = static_cast<unsigned int*>(newData.buffer);
	newData.shader = reinterpret_cast<ShaderData*>(newData.freeList + required);

	if (data.buffer != nullptr)
	{
		// Since the whole freelist needs to be copied, combine copies of freeList and material
		// Aligment of MaterialData is 8 bytes, allocated needs to be an even number
		size_t copyBytes = data.allocated * sizeof(unsigned int) + data.count * sizeof(ShaderData);
		std::memcpy(newData.buffer, data.buffer, copyBytes);

		allocator->Deallocate(data.buffer);
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
		id.i = data.count;
	}
	else
	{
		id.i = freeListFirst;
		freeListFirst = data.freeList[freeListFirst];
	}

	data.shader[id.i].buffer = nullptr;
	data.shader[id.i].uniformBlockDefinition = StringRef();
	data.shader[id.i].transparencyType = TransparencyType::Opaque;
	data.shader[id.i].driverId = 0;
	data.shader[id.i].uniforms = UniformList();

	++data.count;

	return id;
}

void ShaderManager::RemoveShader(ShaderId id)
{
	{
		HashMap<uint32_t, ShaderId>::Iterator itr = nameHashMap.Begin();
		HashMap<uint32_t, ShaderId>::Iterator end = nameHashMap.End();
		for (; itr != end; ++itr)
			if (itr->second == id)
				break;

		// TODO: Fix this hack, make removing by value easier
		if (itr != end)
			nameHashMap.Remove(&*itr);
	}

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

ShaderId ShaderManager::GetIdByPath(StringRef path)
{
	uint32_t hash = Hash::FNV1a_32(path.str, path.len);

	HashMap<uint32_t, ShaderId>::KeyValuePair* pair = nameHashMap.Lookup(hash);
	if (pair != nullptr)
		return pair->second;

	if (data.count == data.allocated)
		this->Reallocate(data.count + 1);

	Buffer<char> file(allocator);
	String pathStr(allocator, path);

	if (File::ReadText(pathStr.GetCStr(), file))
	{
		ShaderId id = CreateShader();
		ShaderData& shader = data.shader[id.i];

		if (ShaderLoader::LoadFromConfiguration(shader, file.GetRef(), allocator, renderDevice))
		{
			pair = nameHashMap.Insert(hash);
			pair->second = id;

			return id;
		}
		else
		{
			RemoveShader(id);
		}
	}
	else
	{
		return ShaderId{};
	}
}
