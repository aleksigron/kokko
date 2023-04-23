#include "Rendering/RenderGraphResources.hpp"

#include "Core/Core.hpp"

#include "Rendering/CascadedShadowMap.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/RenderTypes.hpp"

#include "Resources/MeshManager.hpp"
#include "Resources/MeshPresets.hpp"

namespace kokko
{

RenderGraphResources::RenderGraphResources(kokko::render::Device* renderDevice, MeshManager* meshManager) :
	renderDevice(renderDevice),
	meshManager(meshManager)
{
	framebufferGbuffer.SetRenderDevice(renderDevice);
	framebufferShadow.SetRenderDevice(renderDevice);
	framebufferLightAccumulation.SetRenderDevice(renderDevice);
	framebufferAmbientOcclusion.SetRenderDevice(renderDevice);
}

void RenderGraphResources::VerifyResourcesAreCreated(Vec2i fullscreenViewportResolution)
{
	KOKKO_PROFILE_FUNCTION();

	auto scope = renderDevice->CreateDebugScope(0, kokko::ConstStringView("RenderGraph_InitResources"));

	// Create resolution invariant resources

	if (framebufferShadow.GetFramebufferId() == 0)
	{
		int shadowSide = CascadedShadowMap::GetShadowCascadeResolution();
		unsigned int shadowCascadeCount = CascadedShadowMap::GetCascadeCount();
		Vec2i size(shadowSide * shadowCascadeCount, shadowSide);

		RenderTextureSizedFormat depthFormat = RenderTextureSizedFormat::D32F;
		framebufferShadow.Create(size.x, size.y, depthFormat, ArrayView<RenderTextureSizedFormat>());
		// TODO: use sampler to set shadowmap comparison mode

		framebufferShadow.SetDebugLabel(kokko::ConstStringView("Renderer shadow framebuffer"));
	}

	// Delete wrong size framebuffers

	if (this->fullscreenViewportSize != fullscreenViewportResolution)
	{
		framebufferGbuffer.Destroy();
		framebufferLightAccumulation.Destroy();
		framebufferAmbientOcclusion.Destroy();
	}

	// Create correct size framebuffers

	if (fullscreenViewportResolution.x != framebufferGbuffer.GetWidth() ||
		fullscreenViewportResolution.y != framebufferGbuffer.GetHeight())
	{
		fullscreenViewportSize = fullscreenViewportResolution;

		int width = fullscreenViewportResolution.x;
		int height = fullscreenViewportResolution.y;

		{
			// G-buffer

			RenderTextureSizedFormat depthFormat = RenderTextureSizedFormat::D32F;

			RenderTextureSizedFormat colorFormats[GbufferColorCount];
			colorFormats[GbufferAlbedoIndex] = RenderTextureSizedFormat::SRGB8;
			colorFormats[GbufferNormalIndex] = RenderTextureSizedFormat::RG16;
			colorFormats[GbufferMaterialIndex] = RenderTextureSizedFormat::RGB8;

			ArrayView<RenderTextureSizedFormat> colorFormatsList(colorFormats, GbufferColorCount);

			framebufferGbuffer.Create(width, height, depthFormat, colorFormatsList);
			framebufferGbuffer.SetDebugLabel(ConstStringView("Geometry framebuffer"));
		}

		{
			// HDR light accumulation framebuffer

			RenderTextureSizedFormat colorFormat = RenderTextureSizedFormat::RGB16F;
			ArrayView<RenderTextureSizedFormat> colorFormatList(&colorFormat, 1);
			framebufferLightAccumulation.Create(width, height, Optional<RenderTextureSizedFormat>(), colorFormatList);
			framebufferLightAccumulation.AttachExternalDepthTexture(framebufferGbuffer.GetDepthTextureId());
			framebufferLightAccumulation.SetDebugLabel(ConstStringView("Light accumulation framebuffer"));
		}

		{
			// Ambient occlusion framebuffer

			RenderTextureSizedFormat colorFormat = RenderTextureSizedFormat::R8;
			ArrayView<RenderTextureSizedFormat> colorFormatList(&colorFormat, 1);
			framebufferAmbientOcclusion.Create(width, height, Optional<RenderTextureSizedFormat>(), colorFormatList);
			framebufferAmbientOcclusion.SetDebugLabel(ConstStringView("Ambient occlusion framebuffer"));
		}
	}
}

void RenderGraphResources::Deinitialize()
{
	framebufferShadow.Destroy();
	framebufferGbuffer.Destroy();
	framebufferLightAccumulation.Destroy();
	framebufferAmbientOcclusion.Destroy();
}

const render::Framebuffer& RenderGraphResources::GetGeometryBuffer() const
{
	return framebufferGbuffer;
}

const render::Framebuffer& RenderGraphResources::GetShadowBuffer() const
{
	return framebufferShadow;
}

const render::Framebuffer& RenderGraphResources::GetLightAccumulationBuffer() const
{
	return framebufferLightAccumulation;
}

const render::Framebuffer& RenderGraphResources::GetAmbientOcclusionBuffer() const
{
	return framebufferAmbientOcclusion;
}

render::TextureId RenderGraphResources::GetGeometryBufferAlbedoTexture() const
{
	return framebufferGbuffer.GetColorTextureId(GbufferAlbedoIndex);
}

render::TextureId RenderGraphResources::GetGeometryBufferNormalTexture() const
{
	return framebufferGbuffer.GetColorTextureId(GbufferNormalIndex);
}

render::TextureId RenderGraphResources::GetGeometryBufferMaterialTexture() const
{
	return framebufferGbuffer.GetColorTextureId(GbufferMaterialIndex);
}

Vec2i RenderGraphResources::GetFullscreenViewportSize() const
{
	return fullscreenViewportSize;
}

} // namespace kokko
