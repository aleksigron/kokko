#include "Rendering/PostProcessRenderer.hpp"

#include "Core/Core.hpp"

#include "Rendering/PostProcessRenderPass.hpp"
#include "Rendering/CommandEncoder.hpp"
#include "Rendering/Uniform.hpp"

#include "Resources/ModelManager.hpp"
#include "Resources/MeshPresets.hpp"
#include "Resources/ShaderManager.hpp"

namespace kokko
{

PostProcessRenderer::PostProcessRenderer(
	kokko::render::CommandEncoder* encoder,
	ModelManager* modelManager,
	ShaderManager* shaderManager,
	RenderTargetContainer* renderTargetContainer):
	encoder(encoder),
	modelManager(modelManager),
	shaderManager(shaderManager),
	renderTargetContainer(renderTargetContainer)
{
	fullscreenMeshId = ModelId::Null;
}

PostProcessRenderer::~PostProcessRenderer()
{
	Deinitialize();
}

void PostProcessRenderer::Initialize()
{
	KOKKO_PROFILE_FUNCTION();

	fullscreenMeshId = MeshPresets::CreatePlane(modelManager);
}

void PostProcessRenderer::Deinitialize()
{
	if (fullscreenMeshId != ModelId::Null)
	{
		modelManager->RemoveModel(fullscreenMeshId);
		fullscreenMeshId = ModelId::Null;
	}
}

void PostProcessRenderer::RenderPass(const PostProcessRenderPass& pass)
{
	RenderPasses(1, &pass);
}

void PostProcessRenderer::RenderPasses(unsigned int count, const PostProcessRenderPass* passes)
{
	KOKKO_PROFILE_FUNCTION();

	auto& mesh = modelManager->GetModelMeshes(fullscreenMeshId)[0];
	auto& prim = modelManager->GetModelPrimitives(fullscreenMeshId)[0];
	encoder->BindVertexArray(prim.vertexArrayId);

	for (unsigned int i = 0; i < count; ++i)
	{
		const PostProcessRenderPass& pass = passes[i];

		if (pass.enableBlending)
		{
			encoder->BlendingEnable();
			encoder->BlendFunction(pass.sourceBlendFactor, pass.destinationBlendFactor);
		}
		else
		{
			encoder->BlendingDisable();
		}

		encoder->SetViewport(0, 0, pass.viewportSize.x, pass.viewportSize.y);

		encoder->BindFramebuffer(pass.framebufferId);

		const ShaderData& shader = shaderManager->GetShaderData(pass.shaderId);
		encoder->UseShaderProgram(shader.driverId);

		if (pass.uniformBufferId != kokko::render::BufferId(0))
		{
			encoder->BindBufferRange(RenderBufferTarget::UniformBuffer,
				pass.uniformBindingPoint, pass.uniformBufferId,
				pass.uniformBufferRangeStart, pass.uniformBufferRangeSize);
		}

		if (pass.textureCount > 0)
			BindTextures(shader, pass.textureCount, pass.textureNameHashes, pass.textureIds, pass.samplerIds);

		encoder->DrawIndexed(mesh.primitiveMode, mesh.indexType, prim.count, 0, 0);
	}
}

void PostProcessRenderer::BindTextures(const ShaderData& shader, unsigned int count,
	const uint32_t* nameHashes, const render::TextureId* textures, const render::SamplerId* samplers)
{
	for (unsigned int i = 0; i < count; ++i)
	{
		encoder->BindSampler(i, samplers[i]);

		const kokko::TextureUniform* tu = shader.uniforms.FindTextureUniformByNameHash(nameHashes[i]);
		if (tu != nullptr)
		{
			encoder->BindTextureToShader(tu->uniformLocation, i, textures[i]);
		}
	}
}

} // namespace kokko
