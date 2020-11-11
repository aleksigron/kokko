#pragma once

#include <cstdint>

#include "Core/HashMap.hpp"
#include "Core/StringRef.hpp"

#include "Resources/MaterialData.hpp"

#include "Rendering/TransparencyType.hpp"

class Allocator;

struct MaterialData
{
	TransparencyType transparency;
	unsigned int shader;
	unsigned int uniformCount;
	unsigned int uniformDataSize;
	unsigned char* uniformData;
	MaterialUniform uniforms[Shader::MaxMaterialUniforms];
};

class MaterialManager
{
private:
	Allocator* allocator;

	struct InstanceData
	{
		unsigned int count;
		unsigned int allocated;
		void *buffer;

		unsigned int* freeList;
		MaterialData* material;
	}
	data;

	unsigned int freeListFirst;
	HashMap<uint32_t, MaterialId> nameHashMap;

	void Reallocate(unsigned int required);

	bool LoadFromConfiguration(MaterialId id, char* config);

	void SetShader(MaterialId id, const Shader* shader);

public:
	MaterialManager(Allocator* allocator);
	~MaterialManager();

	MaterialId CreateMaterial();
	void RemoveMaterial(MaterialId id);

	MaterialId GetIdByPath(StringRef path);
	MaterialId GetIdByPathHash(uint32_t pathHash)
	{
		auto pair = nameHashMap.Lookup(pathHash);
		return pair != nullptr ? pair->second : MaterialId{};
	}

	const MaterialData& GetMaterialData(MaterialId id) const
	{
		return data.material[id.i];
	}
};
