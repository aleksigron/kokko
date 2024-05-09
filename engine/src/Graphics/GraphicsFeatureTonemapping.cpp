#include "Graphics/GraphicsFeatureTonemapping.hpp"

#include "Core/Core.hpp"

#include "Graphics/EnvironmentSystem.hpp"
#include "Graphics/GraphicsFeatureCommandList.hpp"

#include "Rendering/CameraSystem.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/RenderGraphResources.hpp"
#include "Rendering/RenderPassType.hpp"
#include "Rendering/PostProcessRenderer.hpp"
#include "Rendering/PostProcessRenderPass.hpp"
#include "Rendering/StaticUniformBuffer.hpp"

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

	kokko::render::Device* device = parameters.renderDevice;

	{
		device->CreateBuffers(1, &uniformBufferId);
		device->SetBufferStorage(uniformBufferId, sizeof(TonemapUniformBlock), nullptr, BufferStorageFlags::Dynamic);

		kokko::ConstStringView label("Tonemapping uniform buffer");
		device->SetObjectLabel(RenderObjectType::Buffer, uniformBufferId.i, label);
	}

	{
		ConstStringView path("engine/shaders/post_process/tonemap.glsl");
		shaderId = parameters.shaderManager->FindShaderByPath(path); 
	}
}

void GraphicsFeatureTonemapping::Deinitialize(const InitializeParameters& parameters)
{
	parameters.renderDevice->DestroyBuffers(1, &uniformBufferId);
	uniformBufferId = render::BufferId();
}

void GraphicsFeatureTonemapping::Upload(const UploadParameters& parameters)
{
	TonemapUniformBlock uniforms;
	uniforms.exposure = 1.0f;

	Entity cameraEntity = parameters.cameraSystem->GetActiveCamera();
	if (cameraEntity != Entity::Null)
	{
		kokko::CameraId cameraId = parameters.cameraSystem->Lookup(cameraEntity);

		if (cameraId != kokko::CameraId::Null)
			uniforms.exposure = parameters.cameraSystem->GetExposure(cameraId);
	}

	kokko::render::Device* device = parameters.renderDevice;
	device->SetBufferSubData(uniformBufferId, 0, sizeof(TonemapUniformBlock), &uniforms);
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
	pass.samplerIds[0] = render::SamplerId();
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
