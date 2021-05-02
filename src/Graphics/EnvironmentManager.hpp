#pragma once

#include "Core/Array.hpp"

#include "Resources/MeshData.hpp"
#include "Resources/TextureId.hpp"

class Allocator;
class RenderDevice;
class ShaderManager;
class MeshManager;
class TextureManager;

struct EnvironmentTextures
{
	TextureId environmentTexture;
	TextureId diffuseIrradianceTexture;
	TextureId specularIrradianceTexture;
};

class EnvironmentManager
{
private:
	static const size_t CubemapSideCount = 6;
	static const size_t SpecularMipmapLevelCount = 6;

	Allocator* allocator;
	RenderDevice* renderDevice;
	ShaderManager* shaderManager;
	MeshManager* meshManager;
	TextureManager* textureManager;

	Array<EnvironmentTextures> environmentMaps;

	EnvironmentTextures emptyEnvironmentMap;

	size_t viewportBlockStride;
	size_t specularBlockStride;

	unsigned int framebufferId;
	unsigned int viewportUniformBufferId;
	unsigned int specularUniformBufferId;
	unsigned int samplerId;
	MeshId cubeMeshId;

	void LoadEmptyEnvironmentMap();

public:
	EnvironmentManager(Allocator* allocator, RenderDevice* renderDevice,
		ShaderManager* shaderManager, MeshManager* meshManager, TextureManager* textureManager);
	EnvironmentManager(const EnvironmentManager&) = delete;
	EnvironmentManager(EnvironmentManager&&) = delete;
	~EnvironmentManager();

	EnvironmentManager& operator=(const EnvironmentManager&) = delete;
	EnvironmentManager& operator=(EnvironmentManager&&) = delete;

	void Initialize();
	void Deinitialize();

	int LoadHdrEnvironmentMap(const char* equirectMapPath);

	EnvironmentTextures GetEnvironmentMap(int environmentId) const { return environmentMaps[environmentId]; }
	EnvironmentTextures GetEmptyEnvironmentMap() const { return emptyEnvironmentMap; }
};
