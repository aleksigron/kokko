#pragma once

#include <cstdint>

#include "Core/Array.hpp"
#include "Core/HashMap.hpp"
#include "Core/StringRef.hpp"

#include "Rendering/Uniform.hpp"
#include "Rendering/TransparencyType.hpp"

#include "Resources/MaterialData.hpp"
#include "Resources/ShaderId.hpp"

class Allocator;
class ShaderManager;
class TextureManager;
class RenderDevice;

struct MaterialData
{
	TransparencyType transparency;
	ShaderId shaderId;
	unsigned int cachedShaderDeviceId;

	unsigned int uniformBufferObject;

	void* buffer;
	unsigned char* uniformData;

	UniformList uniforms;

	char* materialPath;
};

class MaterialManager
{
private:
	Allocator* allocator;
	RenderDevice* renderDevice;
	ShaderManager* shaderManager;
	TextureManager* textureManager;

	Array<unsigned char> uniformScratchBuffer;

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
	HashMap<uint32_t, MaterialId> pathHashMap;

	void Reallocate(unsigned int required);

	bool LoadFromConfiguration(MaterialId id, char* config);

	void SetShader(MaterialId id, ShaderId shaderId);

public:
	MaterialManager(
		Allocator* allocator,
		RenderDevice* renderDevice,
		ShaderManager* shaderManager,
		TextureManager* textureManager);

	~MaterialManager();

	MaterialId CreateMaterial();
	void RemoveMaterial(MaterialId id);

	MaterialId CreateCopy(MaterialId copyFrom);

	MaterialId GetIdByPath(StringRef path);
	MaterialId GetIdByPathHash(uint32_t pathHash);

	const MaterialData& GetMaterialData(MaterialId id) const { return data.material[id.i]; }
	MaterialData& GetMaterialData(MaterialId id) { return data.material[id.i]; }

	void UpdateUniformsToGPU(MaterialId id);
};
