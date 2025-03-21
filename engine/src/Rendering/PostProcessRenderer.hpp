#pragma once

#include <cstdint>

#include "Resources/MeshId.hpp"

#include "Rendering/RenderResourceId.hpp"

namespace kokko
{

class ModelManager;
class ShaderManager;
class RenderTargetContainer;
struct PostProcessRenderPass;
struct ShaderData;

namespace render
{
class CommandEncoder;
}

class PostProcessRenderer
{
private:
	render::CommandEncoder* encoder;
	ModelManager* modelManager;
	ShaderManager* shaderManager;
	RenderTargetContainer* renderTargetContainer;

	ModelId fullscreenMeshId;

	void BindTextures(const ShaderData& shader, unsigned int count,
		const uint32_t* nameHashes, const render::TextureId* textures, const render::SamplerId* samplers);

public:
	PostProcessRenderer(render::CommandEncoder* encoder, ModelManager* modelManager,
		ShaderManager* shaderManager, RenderTargetContainer* renderTargetContainer);
	~PostProcessRenderer();

	void Initialize();
	void Deinitialize();

	RenderTargetContainer* GetRenderTargetContainer() { return renderTargetContainer; }

	void RenderPass(const PostProcessRenderPass& pass);
	void RenderPasses(unsigned int count, const PostProcessRenderPass* passes);
};

} // namespace kokko
