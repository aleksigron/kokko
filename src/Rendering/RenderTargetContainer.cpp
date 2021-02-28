#include "Rendering/RenderTargetContainer.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/RenderDevice.hpp"

RenderTargetContainer::RenderTargetContainer(Allocator* allocator, RenderDevice* renderDevice) :
	allocator(allocator),
	renderDevice(renderDevice),
	renderTargetCount(0)
{
	void* buffer = allocator->Allocate(sizeof(RenderTarget) * MaxRenderTargetCount);
	renderTargets = static_cast<RenderTarget*>(buffer);
}

RenderTargetContainer::~RenderTargetContainer()
{
	allocator->Deallocate(renderTargets);
}

RenderTarget* RenderTargetContainer::AcquireRenderTarget(Vec2i size, RenderTextureSizedFormat format)
{
	RenderTarget* result = nullptr;

	for (size_t i = 0; i < renderTargetCount; ++i)
	{
		RenderTarget* target = &renderTargets[i];
		if (target->inUse == false && target->colorFormat == format &&
			target->size.x == size.x && target->size.y == size.y)
		{
			target->inUse = true;
			result = target;
		}
	}

	if (result == nullptr && renderTargetCount < MaxRenderTargetCount)
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

		result = &renderTargets[renderTargetCount];
		renderTargetCount += 1;

		result->size = size;
		result->colorFormat = format;
		result->colorTexture = texture;
		result->framebuffer = framebuffer;
		result->inUse = true;
	}

	return result;
}

void RenderTargetContainer::ReleaseRenderTarget(RenderTarget* target)
{
	target->inUse = false;
}
