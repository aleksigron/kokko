#pragma once

#include <cstdint>

#include "Core/Array.hpp"
#include "Core/HashMap.hpp"
#include "Core/StringRef.hpp"
#include "Core/Uid.hpp"

#include "Rendering/UniformData.hpp"
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

class MaterialManager
{
private:
	struct MaterialData
	{
		kokko::Uid uid;
		TransparencyType transparency;
		ShaderId shaderId;
		unsigned int cachedShaderDeviceId;

		unsigned int uniformBufferObject;

		kokko::UniformData uniformData;
	};

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

	MaterialId FindMaterialByUid(const kokko::Uid& uid);
	MaterialId FindMaterialByPath(const StringRef& path);

	kokko::Uid GetMaterialUid(MaterialId id) const;

	TransparencyType GetMaterialTransparency(MaterialId id) const;
	void SetMaterialTransparency(MaterialId id, TransparencyType transparency);

	ShaderId GetMaterialShader(MaterialId id) const;
	void SetMaterialShader(MaterialId id, ShaderId shader);

	const kokko::UniformData& GetMaterialUniforms(MaterialId id) const
	{ return data.material[id.i].uniformData; }
	kokko::UniformData& GetMaterialUniforms(MaterialId id)
	{ return data.material[id.i].uniformData; }

	unsigned int GetMaterialShaderDeviceId(MaterialId id) const
	{ return data.material[id.i].cachedShaderDeviceId; }
	unsigned int GetMaterialUniformBufferId(MaterialId id) const
	{ return data.material[id.i].uniformBufferObject; }

	void UpdateUniformsToGPU(MaterialId id);
};
