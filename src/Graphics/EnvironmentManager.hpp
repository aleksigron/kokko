#pragma once

#include "Core/Array.hpp"

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
};

class EnvironmentManager
{
private:
	static const size_t CubemapSideCount = 6;

	Allocator* allocator;
	RenderDevice* renderDevice;
	ShaderManager* shaderManager;
	MeshManager* meshManager;
	TextureManager* textureManager;

	Array<EnvironmentTextures> environmentMaps;

	size_t blockStride;

	unsigned int framebufferId;
	unsigned int viewportUniformBufferId;
	unsigned int samplerId;

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
};
