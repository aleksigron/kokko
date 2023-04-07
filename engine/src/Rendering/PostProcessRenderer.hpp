#pragma once

#include <cstdint>

#include "Resources/MeshId.hpp"

#include "Rendering/RenderResourceId.hpp"

class RenderDevice;
class MeshManager;
class ShaderManager;
class RenderTargetContainer;

struct ShaderData;
struct PostProcessRenderPass;

namespace kokko
{
namespace render
{
class CommandEncoder;
}
}

class PostProcessRenderer
{
private:
	kokko::render::CommandEncoder* encoder;
	MeshManager* meshManager;
	ShaderManager* shaderManager;
	RenderTargetContainer* renderTargetContainer;

	MeshId fullscreenMeshId;

	void BindTextures(const ShaderData& shader, unsigned int count,
		const uint32_t* nameHashes, const kokko::RenderTextureId* textures, const kokko::RenderSamplerId* samplers);

public:
	PostProcessRenderer(kokko::render::CommandEncoder* encoder, MeshManager* meshManager,
		ShaderManager* shaderManager, RenderTargetContainer* renderTargetContainer);
	~PostProcessRenderer();

	void Initialize();
	void Deinitialize();

	RenderTargetContainer* GetRenderTargetContainer() { return renderTargetContainer; }

	void RenderPass(const PostProcessRenderPass& pass);
	void RenderPasses(unsigned int count, const PostProcessRenderPass* passes);
};
