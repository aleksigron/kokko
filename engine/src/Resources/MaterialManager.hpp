#pragma once

#include <cstdint>

#include "Core/Array.hpp"
#include "Core/HashMap.hpp"
#include "Core/StringView.hpp"
#include "Core/Uid.hpp"

#include "Rendering/UniformData.hpp"
#include "Rendering/TransparencyType.hpp"

#include "Resources/MaterialData.hpp"
#include "Resources/ShaderId.hpp"

namespace kokko
{
class Allocator;
class AssetLoader;
class MaterialSerializer;
class ShaderManager;
class TextureManager;

namespace render
{
class Device;
}

class MaterialManager
{
private:
	struct MaterialData
	{
		kokko::Uid uid;
		TransparencyType transparency;
		ShaderId shaderId;
		kokko::render::ShaderId cachedShaderDeviceId;

		kokko::render::BufferId uniformBufferObject;

		kokko::UniformData uniformData;
	};

	Allocator* allocator;
	kokko::AssetLoader* assetLoader;
	kokko::render::Device* renderDevice;
	kokko::ShaderManager* shaderManager;
	TextureManager* textureManager;

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
	HashMap<kokko::Uid, MaterialId> uidMap;

	void Reallocate(unsigned int required);

	friend class kokko::MaterialSerializer;

public:
	MaterialManager(
		Allocator* allocator,
		kokko::AssetLoader* assetLoader,
		kokko::render::Device* renderDevice,
		kokko::ShaderManager* shaderManager,
		TextureManager* textureManager);

	~MaterialManager();

	MaterialId CreateMaterial();
	void RemoveMaterial(MaterialId id);

	MaterialId FindMaterialByUid(const kokko::Uid& uid);
	MaterialId FindMaterialByPath(kokko::ConstStringView path);

	kokko::Uid GetMaterialUid(MaterialId id) const;

	TransparencyType GetMaterialTransparency(MaterialId id) const;
	void SetMaterialTransparency(MaterialId id, TransparencyType transparency);

	ShaderId GetMaterialShader(MaterialId id) const;
	void SetMaterialShader(MaterialId id, ShaderId shader);

	const kokko::UniformData& GetMaterialUniforms(MaterialId id) const
	{ return data.material[id.i].uniformData; }
	kokko::UniformData& GetMaterialUniforms(MaterialId id)
	{ return data.material[id.i].uniformData; }

	kokko::render::ShaderId GetMaterialShaderDeviceId(MaterialId id) const
	{ return data.material[id.i].cachedShaderDeviceId; }
	kokko::render::BufferId GetMaterialUniformBufferId(MaterialId id) const
	{ return data.material[id.i].uniformBufferObject; }

	void UpdateUniformsToGPU(MaterialId id);
};

} // namespace kokko
