#include "Rendering/PostProcessRenderer.hpp"

#include "Rendering/PostProcessRenderPass.hpp"
#include "Rendering/RenderCommandData.hpp"
#include "Rendering/RenderDevice.hpp"

#include "Resources/MeshManager.hpp"
#include "Resources/MeshPresets.hpp"
#include "Resources/ShaderManager.hpp"

PostProcessRenderer::PostProcessRenderer(
	RenderDevice* renderDevice,
	MeshManager* meshManager,
	ShaderManager* shaderManager):
	renderDevice(renderDevice),
	meshManager(meshManager),
	shaderManager(shaderManager)
{
	fullscreenMeshId = MeshId{ 0 };
}

PostProcessRenderer::~PostProcessRenderer()
{
	Deinitialize();
}

void PostProcessRenderer::Initialize()
{
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
	RenderCommandData::BindFramebufferData bindFramebufferCommand;
	bindFramebufferCommand.target = RenderFramebufferTarget::Framebuffer;
	bindFramebufferCommand.framebuffer = pass.framebufferId;
	renderDevice->BindFramebuffer(&bindFramebufferCommand);

	RenderCommandData::ViewportData viewport{
		0, 0, pass.viewportSize.x, pass.viewportSize.y
	};
	renderDevice->Viewport(&viewport);

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
		BindTextures(shader, pass.textureCount, pass.textureNameHashes, pass.textureIds);

	const MeshDrawData* draw = meshManager->GetDrawData(fullscreenMeshId);
	renderDevice->BindVertexArray(draw->vertexArrayObject);
	renderDevice->DrawIndexed(draw->primitiveMode, draw->count, draw->indexType);
}

void PostProcessRenderer::BindTextures(const ShaderData& shader, unsigned int count,
	const uint32_t* nameHashes, const unsigned int* textures)
{
	for (unsigned int i = 0; i < count; ++i)
	{
		const TextureUniform* tu = shader.uniforms.FindTextureUniformByNameHash(nameHashes[i]);
		if (tu != nullptr)
		{
			renderDevice->SetUniformInt(tu->uniformLocation, i);
			renderDevice->SetActiveTextureUnit(i);
			renderDevice->BindTexture(RenderTextureTarget::Texture2d, textures[i]);
		}
	}
}
