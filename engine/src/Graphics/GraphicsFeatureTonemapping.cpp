#include "Graphics/GraphicsFeatureTonemapping.hpp"

#include "Core/Core.hpp"

#include "Graphics/GraphicsFeatureCommandList.hpp"

#include "Rendering/RenderDevice.hpp"
#include "Rendering/RenderGraphResources.hpp"
#include "Rendering/RenderPassType.hpp"
#include "Rendering/PostProcessRenderer.hpp"
#include "Rendering/PostProcessRenderPass.hpp"
#include "Rendering/StaticUniformBuffer.hpp"

#include "Resources/MeshManager.hpp"
#include "Resources/ShaderManager.hpp"

namespace kokko
{

namespace
{

struct TonemapUniformBlock
{
	alignas(16) float exposure;
};

} // Anonymous namespace

GraphicsFeatureTonemapping::GraphicsFeatureTonemapping() :
	renderOrder(0),
	shaderId(ShaderId::Null),
	uniformBufferId(0)
{
}

void GraphicsFeatureTonemapping::SetOrder(unsigned int order)
{
	renderOrder = order;
}

void GraphicsFeatureTonemapping::Initialize(const InitializeParameters& parameters)
{
	KOKKO_PROFILE_FUNCTION();

	RenderDevice* device = parameters.renderDevice;

	{
		device->CreateBuffers(1, &uniformBufferId);
		device->BindBuffer(RenderBufferTarget::UniformBuffer, uniformBufferId);

		RenderCommandData::SetBufferStorage storage{};
		storage.target = RenderBufferTarget::UniformBuffer;
		storage.size = sizeof(TonemapUniformBlock);
		storage.data = nullptr;
		storage.dynamicStorage = true;
		device->SetBufferStorage(&storage);

		kokko::ConstStringView label("Tonemapping uniform buffer");
		device->SetObjectLabel(RenderObjectType::Buffer, uniformBufferId, label);
	}

	{
		ConstStringView path("engine/shaders/post_process/tonemap.glsl");
		shaderId = parameters.shaderManager->FindShaderByPath(path); 
	}
}

void GraphicsFeatureTonemapping::Deinitialize(const InitializeParameters& parameters)
{
	parameters.renderDevice->DestroyBuffers(1, &uniformBufferId);
	uniformBufferId = 0;
}

void GraphicsFeatureTonemapping::Upload(const UploadParameters& parameters)
{
	RenderDevice* device = parameters.renderDevice;

	TonemapUniformBlock uniforms;
	uniforms.exposure = 1.0f;

	device->BindBuffer(RenderBufferTarget::UniformBuffer, uniformBufferId);
	device->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, sizeof(TonemapUniformBlock), &uniforms);
}

void GraphicsFeatureTonemapping::Submit(const SubmitParameters& parameters)
{
	parameters.commandList.AddToFullscreenViewportWithOrder(RenderPassType::PostProcess, renderOrder, 0);
}

void GraphicsFeatureTonemapping::Render(const RenderParameters& parameters)
{
	KOKKO_PROFILE_FUNCTION();

	const RenderGraphResources* resources = parameters.renderGraphResources;

	PostProcessRenderPass pass;

	pass.textureNameHashes[0] = "light_acc_map"_hash;
	pass.textureIds[0] = resources->GetLightAccumulationBuffer().GetColorTextureId(0);
	pass.samplerIds[0] = 0;
	pass.textureCount = 1;

	pass.uniformBufferId = uniformBufferId;
	pass.uniformBindingPoint = UniformBlockBinding::Object;
	pass.uniformBufferRangeStart = 0;
	pass.uniformBufferRangeSize = sizeof(TonemapUniformBlock);

	pass.framebufferId = parameters.finalTargetFramebufferId;
	pass.viewportSize = parameters.fullscreenViewport.viewportRectangle.size;
	pass.shaderId = shaderId;
	pass.enableBlending = false;

	parameters.postProcessRenderer->RenderPasses(1, &pass);
}

}
