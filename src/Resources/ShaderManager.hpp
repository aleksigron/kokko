#pragma once

#include <cstdint>

#include "Core/HashMap.hpp"
#include "Core/StringRef.hpp"

#include "Rendering/Uniform.hpp"
#include "Rendering/TransparencyType.hpp"

#include "Resources/ShaderId.hpp"

class Allocator;
class Filesystem;
class RenderDevice;

struct ShaderData
{
	void* buffer;
	StringRef uniformBlockDefinition;

	TransparencyType transparencyType;

	unsigned int driverId;

	UniformList uniforms;
};

class ShaderManager
{
private:
	Allocator* allocator;
	Filesystem* filesystem;
	RenderDevice* renderDevice;

	struct InstanceData
	{
		unsigned int count;
		unsigned int allocated;
		void* buffer;

		unsigned int* freeList;
		ShaderData* shader;
	}
	data;

	unsigned int freeListFirst;
	HashMap<uint32_t, ShaderId> nameHashMap;

	void Reallocate(unsigned int required);

public:
	ShaderManager(Allocator* allocator, Filesystem* filesystem, RenderDevice* renderDevice);
	~ShaderManager();

	ShaderId CreateShader();
	void RemoveShader(ShaderId id);

	ShaderId GetIdByPath(StringRef path);
	ShaderId GetIdByPathHash(uint32_t pathHash)
	{
		auto pair = nameHashMap.Lookup(pathHash);
		return pair != nullptr ? pair->second : ShaderId{};
	}

	const ShaderData& GetShaderData(ShaderId id) const
	{
		return data.shader[id.i];
	}
};
