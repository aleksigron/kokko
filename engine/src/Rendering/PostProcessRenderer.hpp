#pragma once

#include <cstdint>

#include "Resources/MeshId.hpp"

#include "Rendering/RenderResourceId.hpp"

class MeshManager;
class RenderTargetContainer;

namespace kokko
{

class ShaderManager;
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
	MeshManager* meshManager;
	ShaderManager* shaderManager;
	RenderTargetContainer* renderTargetContainer;

	MeshId fullscreenMeshId;

	void BindTextures(const ShaderData& shader, unsigned int count,
		const uint32_t* nameHashes, const render::TextureId* textures, const render::SamplerId* samplers);

public:
	PostProcessRenderer(render::CommandEncoder* encoder, MeshManager* meshManager,
		ShaderManager* shaderManager, RenderTargetContainer* renderTargetContainer);
	~PostProcessRenderer();

	void Initialize();
	void Deinitialize();

	RenderTargetContainer* GetRenderTargetContainer() { return renderTargetContainer; }

	void RenderPass(const PostProcessRenderPass& pass);
	void RenderPasses(unsigned int count, const PostProcessRenderPass* passes);
};

} // namespace kokko
