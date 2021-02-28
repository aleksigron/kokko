#pragma once

#include <cstdint>

#include "Resources/MeshData.hpp"

class RenderDevice;
class MeshManager;
class ShaderManager;

struct ShaderData;
struct PostProcessRenderPass;

class PostProcessRenderer
{
private:
	RenderDevice* renderDevice;
	MeshManager* meshManager;
	ShaderManager* shaderManager;

	MeshId fullscreenMeshId;

	void BindTextures(const ShaderData& shader, unsigned int count,
		const uint32_t* nameHashes, const unsigned int* textures);

public:
	PostProcessRenderer(RenderDevice* renderDevice, MeshManager* meshManager, ShaderManager* shaderManager);
	~PostProcessRenderer();

	void Initialize();
	void Deinitialize();

	void RenderPass(const PostProcessRenderPass& pass);
};
