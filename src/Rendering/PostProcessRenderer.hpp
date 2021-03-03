#pragma once

#include <cstdint>

#include "Resources/MeshData.hpp"

class RenderDevice;
class MeshManager;
class ShaderManager;
class RenderTargetContainer;

struct ShaderData;
struct PostProcessRenderPass;

class PostProcessRenderer
{
private:
	RenderDevice* renderDevice;
	MeshManager* meshManager;
	ShaderManager* shaderManager;
	RenderTargetContainer* renderTargetContainer;

	MeshId fullscreenMeshId;

	void BindTextures(const ShaderData& shader, unsigned int count,
		const uint32_t* nameHashes, const unsigned int* textures, const unsigned int* samplers);

public:
	PostProcessRenderer(RenderDevice* renderDevice, MeshManager* meshManager,
		ShaderManager* shaderManager, RenderTargetContainer* renderTargetContainer);
	~PostProcessRenderer();

	void Initialize();
	void Deinitialize();

	RenderTargetContainer* GetRenderTargetContainer() { return renderTargetContainer; }

	void RenderPass(const PostProcessRenderPass& pass);
	void RenderPasses(unsigned int count, const PostProcessRenderPass* passes);
};
