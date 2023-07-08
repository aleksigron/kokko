#pragma once

#include "Core/Array.hpp"
#include "Core/HashMap.hpp"
#include "Core/Optional.hpp"
#include "Core/String.hpp"
#include "Core/Uid.hpp"

#include "Engine/Entity.hpp"

#include "Rendering/RenderResourceId.hpp"

#include "Resources/MeshId.hpp"
#include "Resources/TextureId.hpp"

class Allocator;

namespace kokko
{

class AssetLoader;
class Filesystem;
class MeshManager;
class ShaderManager;
class TextureManager;

namespace render
{
class CommandEncoder;
class Device;
}

struct EnvironmentId
{
	unsigned int i;

	bool operator==(EnvironmentId other) { return i == other.i; }
	bool operator!=(EnvironmentId other) { return !operator==(other); }

	static const EnvironmentId Null;
};

struct EnvironmentTextures
{
	TextureId environmentTexture;
	TextureId diffuseIrradianceTexture;
	TextureId specularIrradianceTexture;

	EnvironmentTextures() :
		environmentTexture(TextureId::Null),
		diffuseIrradianceTexture(TextureId::Null),
		specularIrradianceTexture(TextureId::Null)
	{
	}
};

class EnvironmentSystem
{
private:
	struct EnvironmentComponent
	{
		Entity entity;
		Optional<Uid> sourceTextureUid;
		EnvironmentTextures textures;
		float exposure;
		bool needsUpload;
	};

	static const uint32_t CubemapSideCount = 6;
	static const uint32_t SpecularMipmapLevelCount = 6;

	Allocator* allocator;
	AssetLoader* assetLoader;
	kokko::render::Device* renderDevice;
	ShaderManager* shaderManager;
	MeshManager* meshManager;
	TextureManager* textureManager;

	Array<EnvironmentComponent> components;
	Array<render::TextureId> texturesToRemove;

	HashMap<unsigned int, EnvironmentId> entityMap;

	EnvironmentTextures emptyEnvironmentMap;

	size_t viewportBlockStride;
	size_t specularBlockStride;

	Array<render::FramebufferId> framebufferIds;
	render::BufferId viewportUniformBufferId;
	render::BufferId specularUniformBufferId;
	render::SamplerId samplerId;
	MeshId cubeMeshId;

	bool resourcesUploaded;

	void LoadEmptyEnvironmentMap();

public:
	EnvironmentSystem(
		Allocator* allocator,
		AssetLoader* assetLoader,
		kokko::render::Device* renderDevice,
		ShaderManager* shaderManager,
		MeshManager* meshManager,
		TextureManager* textureManager);
	EnvironmentSystem(const EnvironmentSystem&) = delete;
	EnvironmentSystem(EnvironmentSystem&&) = delete;
	~EnvironmentSystem();

	EnvironmentSystem& operator=(const EnvironmentSystem&) = delete;
	EnvironmentSystem& operator=(EnvironmentSystem&&) = delete;

	void Initialize();
	void Deinitialize();

	void Upload(render::CommandEncoder* encoder);

	EnvironmentId Lookup(Entity entity);

	EnvironmentId AddComponent(Entity entity);
	void RemoveComponent(EnvironmentId id);
	void RemoveAll();

	void SetEnvironmentTexture(EnvironmentId id, const kokko::Uid& textureUid);
	void SetExposure(EnvironmentId id, float exposure);

	EnvironmentId FindActiveEnvironment();

	EnvironmentTextures GetEnvironmentMap(EnvironmentId id) const;
	Optional<Uid> GetSourceTextureUid(EnvironmentId id) const;
	float GetExposure(EnvironmentId id) const;
	EnvironmentTextures GetEmptyEnvironmentMap() const;
};

} // namespace kokko
