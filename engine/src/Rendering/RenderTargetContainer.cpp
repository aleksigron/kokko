#include "Rendering/RenderTargetContainer.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/RenderDevice.hpp"
#include <cassert>

RenderTargetContainer::RenderTargetContainer(Allocator* allocator, RenderDevice* renderDevice) :
	allocator(allocator),
	renderDevice(renderDevice),
	renderTargetCount(0)
{
}

RenderTargetContainer::~RenderTargetContainer()
{
	DestroyAllRenderTargets();
}

RenderTarget RenderTargetContainer::AcquireRenderTarget(Vec2i size, RenderTextureSizedFormat format)
{
	for (size_t i = 0; i < renderTargetCount; ++i)
	{
		TargetInfo& targetInfo = renderTargets[i];

		if (targetInfo.inUse == false && targetInfo.target.colorFormat == format &&
			targetInfo.target.size.x == size.x && targetInfo.target.size.y == size.y)
		{
			targetInfo.inUse = true;
			
			return targetInfo.target;
		}
	}

	if (renderTargetCount < MaxRenderTargetCount)
	{
		unsigned int framebuffer = 0;
		renderDevice->CreateFramebuffers(1, &framebuffer);
		renderDevice->BindFramebuffer(RenderFramebufferTarget::Framebuffer, framebuffer);

		unsigned int texture = 0;
		renderDevice->CreateTextures(1, &texture);
		renderDevice->BindTexture(RenderTextureTarget::Texture2d, texture);
		renderDevice->SetTextureMinFilter(RenderTextureTarget::Texture2d, RenderTextureFilterMode::Linear);
		renderDevice->SetTextureMagFilter(RenderTextureTarget::Texture2d, RenderTextureFilterMode::Linear);
		renderDevice->SetTextureWrapModeU(RenderTextureTarget::Texture2d, RenderTextureWrapMode::ClampToEdge);
		renderDevice->SetTextureWrapModeV(RenderTextureTarget::Texture2d, RenderTextureWrapMode::ClampToEdge);

		RenderCommandData::SetTextureStorage2D storage{
			RenderTextureTarget::Texture2d, 1, format, size.x, size.y
		};
		renderDevice->SetTextureStorage2D(&storage);

		RenderCommandData::AttachFramebufferTexture2D attachTexture{
			RenderFramebufferTarget::Framebuffer, RenderFramebufferAttachment::Color0,
			RenderTextureTarget::Texture2d, texture, 0
		};
		renderDevice->AttachFramebufferTexture2D(&attachTexture);

		TargetInfo& targetInfo = renderTargets[renderTargetCount];

		targetInfo.target.id = renderTargetCount;
		targetInfo.target.size = size;
		targetInfo.target.colorFormat = format;
		targetInfo.target.colorTexture = texture;
		targetInfo.target.framebuffer = framebuffer;
		targetInfo.inUse = true;

		renderTargetCount += 1;

		return targetInfo.target;
	}

	return RenderTarget{};
}

void RenderTargetContainer::ReleaseRenderTarget(unsigned int renderTargetId)
{
	if (renderTargetId < renderTargetCount)
	{
		renderTargets[renderTargetId].inUse = false;
	}
}

bool RenderTargetContainer::ConfirmAllTargetsAreUnused()
{
	for (size_t i = 0; i < renderTargetCount; ++i)
	{
		assert(renderTargets[i].inUse == false);

		if (renderTargets[i].inUse)
			return false;
	}

	return true;
}

void RenderTargetContainer::DestroyAllRenderTargets()
{
	for (size_t i = 0; i < renderTargetCount; ++i)
	{
		renderDevice->DestroyFramebuffers(1, &renderTargets[i].target.framebuffer);
		renderDevice->DestroyTextures(1, &renderTargets[i].target.colorTexture);

		renderTargets[i].target = RenderTarget{};
		renderTargets[i].inUse = false;
	}

	renderTargetCount = 0;
}
