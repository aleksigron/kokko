#include "Rendering/PostProcessRenderer.hpp"

#include "Core/Core.hpp"

#include "Rendering/PostProcessRenderPass.hpp"
#include "Rendering/RenderCommandData.hpp"
#include "Rendering/RenderDevice.hpp"

#include "Resources/MeshManager.hpp"
#include "Resources/MeshPresets.hpp"
#include "Resources/ShaderManager.hpp"

PostProcessRenderer::PostProcessRenderer(
	RenderDevice* renderDevice,
	MeshManager* meshManager,
	ShaderManager* shaderManager,
	RenderTargetContainer* renderTargetContainer):
	renderDevice(renderDevice),
	meshManager(meshManager),
	shaderManager(shaderManager),
	renderTargetContainer(renderTargetContainer)
{
	fullscreenMeshId = MeshId{ 0 };
}

PostProcessRenderer::~PostProcessRenderer()
{
	Deinitialize();
}

void PostProcessRenderer::Initialize()
{
	KOKKO_PROFILE_FUNCTION();

	fullscreenMeshId = meshManager->CreateMesh();
	MeshPresets::UploadPlane(meshManager, fullscreenMeshId);
}

void PostProcessRenderer::Deinitialize()
{
	if (fullscreenMeshId.IsValid())
	{
		meshManager->RemoveMesh(fullscreenMeshId);
		fullscreenMeshId = MeshId{ 0 };
	}
}

void PostProcessRenderer::RenderPass(const PostProcessRenderPass& pass)
{
	RenderPasses(1, &pass);
}

void PostProcessRenderer::RenderPasses(unsigned int count, const PostProcessRenderPass* passes)
{
	KOKKO_PROFILE_FUNCTION();

	const MeshDrawData* draw = meshManager->GetDrawData(fullscreenMeshId);
	renderDevice->BindVertexArray(draw->vertexArrayObject);

	for (unsigned int i = 0; i < count; ++i)
	{
		const PostProcessRenderPass& pass = passes[i];

		if (pass.enableBlending)
		{
			RenderCommandData::BlendFunctionData blendFunction{
				pass.sourceBlendFactor, pass.destinationBlendFactor
			};

			renderDevice->BlendingEnable();
			renderDevice->BlendFunction(&blendFunction);
		}
		else
		{
			renderDevice->BlendingDisable();
		}

		RenderCommandData::ViewportData viewport{
			0, 0, pass.viewportSize.x, pass.viewportSize.y
		};
		renderDevice->Viewport(&viewport);

		RenderCommandData::BindFramebufferData bindFramebufferCommand{
			RenderFramebufferTarget::Framebuffer, pass.framebufferId
		};
		renderDevice->BindFramebuffer(&bindFramebufferCommand);

		const ShaderData& shader = shaderManager->GetShaderData(pass.shaderId);
		renderDevice->UseShaderProgram(shader.driverId);

		if (pass.uniformBufferId != 0)
		{
			RenderCommandData::BindBufferRange bindBufferRange{
				RenderBufferTarget::UniformBuffer, pass.uniformBindingPoint, pass.uniformBufferId,
				pass.uniformBufferRangeStart, pass.uniformBufferRangeSize
			};

			renderDevice->BindBufferRange(&bindBufferRange);
		}

		if (pass.textureCount > 0)
			BindTextures(shader, pass.textureCount, pass.textureNameHashes, pass.textureIds, pass.samplerIds);

		renderDevice->DrawIndexed(draw->primitiveMode, draw->count, draw->indexType);
	}
}

void PostProcessRenderer::BindTextures(const ShaderData& shader, unsigned int count,
	const uint32_t* nameHashes, const unsigned int* textures, const unsigned int* samplers)
{
	for (unsigned int i = 0; i < count; ++i)
	{
		if (samplers[i] != 0)
			renderDevice->BindSampler(i, samplers[i]);

		const TextureUniform* tu = shader.uniforms.FindTextureUniformByNameHash(nameHashes[i]);
		if (tu != nullptr)
		{
			renderDevice->SetUniformInt(tu->uniformLocation, i);
			renderDevice->SetActiveTextureUnit(i);
			renderDevice->BindTexture(tu->textureTarget, textures[i]);
		}
	}
}
