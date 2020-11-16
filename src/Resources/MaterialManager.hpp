#pragma once

#include <cstdint>

#include "Core/HashMap.hpp"
#include "Core/StringRef.hpp"

#include "Rendering/Uniform.hpp"
#include "Rendering/TransparencyType.hpp"

#include "Resources/MaterialData.hpp"
#include "Resources/ShaderId.hpp"

class Allocator;
class ShaderManager;
class ResourceManager;
class RenderDevice;

struct MaterialData
{
	TransparencyType transparency;
	ShaderId shaderId;
	unsigned int cachedShaderDeviceId;

	unsigned int textureCount;

	unsigned int uniformBufferObject;
	unsigned int uniformBufferSize;

	unsigned int uniformCount;
	unsigned int uniformDataSize;
	unsigned char* uniformData;

	BufferUniform bufferUniforms[ShaderUniform::MaxBufferUniformCount];
	TextureUniform textureUniforms[ShaderUniform::MaxTextureUniformCount];
};

class MaterialManager
{
private:
	Allocator* allocator;
	RenderDevice* renderDevice;
	ShaderManager* shaderManager;
	ResourceManager* resourceManager;

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

	void SetShader(MaterialId id, ShaderId shaderId);

public:
	MaterialManager(
		Allocator* allocator,
		RenderDevice* renderDevice,
		ShaderManager* shaderManager,
		ResourceManager* resourceManager);

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
