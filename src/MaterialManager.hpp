#pragma once

#include <cstdint>

#include "HashMap.hpp"
#include "StringRef.hpp"

#include "MaterialData.hpp"
#include "TransparencyType.hpp"

class Allocator;

struct MaterialUniformData
{
	unsigned int count;
	unsigned int usedData;
	unsigned char* data;
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
		unsigned int* shader;
		MaterialUniformData* uniform;
		TransparencyType* transparency;
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
		return pair != nullptr ? pair->value : MaterialId{};
	}

	unsigned int GetShaderId(MaterialId id) const
	{ return data.shader[id.i]; }

	const MaterialUniformData& GetUniformData(MaterialId id) const
	{ return data.uniform[id.i]; }

	TransparencyType GetTransparency(MaterialId id) const
	{ return data.transparency[id.i]; }
};
