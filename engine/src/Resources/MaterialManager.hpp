#pragma once

#include <cstdint>

#include "Core/Array.hpp"
#include "Core/HashMap.hpp"
#include "Core/StringRef.hpp"
#include "Core/Uid.hpp"

#include "Rendering/Uniform.hpp"
#include "Rendering/TransparencyType.hpp"

#include "Resources/MaterialData.hpp"
#include "Resources/ShaderId.hpp"

namespace kokko
{
class AssetLoader;
}

class Allocator;
class RenderDevice;
class ShaderManager;
class TextureManager;

struct MaterialData
{
	kokko::Uid uid;
	TransparencyType transparency;
	ShaderId shaderId;
	unsigned int cachedShaderDeviceId;

	unsigned int uniformBufferObject;

	void* buffer;
	unsigned char* uniformData;

	UniformList uniforms;
};

class MaterialManager
{
private:
	Allocator* allocator;
	kokko::AssetLoader* assetLoader;
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
	HashMap<kokko::Uid, MaterialId> uidMap;

	void Reallocate(unsigned int required);

	bool LoadFromConfiguration(MaterialId id, StringRef config);

	void SetShader(MaterialId id, ShaderId shaderId);

public:
	MaterialManager(
		Allocator* allocator,
		kokko::AssetLoader* assetLoader,
		RenderDevice* renderDevice,
		ShaderManager* shaderManager,
		TextureManager* textureManager);

	~MaterialManager();

	MaterialId CreateMaterial();
	void RemoveMaterial(MaterialId id);

	MaterialId CreateCopy(MaterialId copyFrom);

	MaterialId FindMaterialByUid(const kokko::Uid& uid);
	MaterialId FindMaterialByPath(const StringRef& path);

	const MaterialData& GetMaterialData(MaterialId id) const;
	MaterialData& GetMaterialData(MaterialId id);

	void UpdateUniformsToGPU(MaterialId id);
};
