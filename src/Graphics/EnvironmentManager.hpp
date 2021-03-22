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
	Allocator* allocator;
	RenderDevice* renderDevice;
	ShaderManager* shaderManager;
	MeshManager* meshManager;
	TextureManager* textureManager;

	Array<EnvironmentTextures> environmentMaps;

public:
	EnvironmentManager(Allocator* allocator, RenderDevice* renderDevice,
		ShaderManager* shaderManager, MeshManager* meshManager, TextureManager* textureManager);
	EnvironmentManager(const EnvironmentManager&) = delete;
	EnvironmentManager(EnvironmentManager&&) = delete;
	~EnvironmentManager();

	EnvironmentManager& operator=(const EnvironmentManager&) = delete;
	EnvironmentManager& operator=(EnvironmentManager&&) = delete;

	int LoadHdrEnvironmentMap(const char* equirectMapPath);

	EnvironmentTextures GetEnvironmentMap(int environmentId) const { return environmentMaps[environmentId]; }
};
