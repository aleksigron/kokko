#include "Rendering/Framebuffer.hpp"

#include <cassert>
#include <utility>

#include "Rendering/RenderDevice.hpp"
#include "Rendering/RenderTypes.hpp"

Framebuffer::Framebuffer() :
	renderDevice(nullptr),
	width(0),
	height(0),
	framebufferId(0),
	colorTextureCount(0),
	depthTextureId(0),
	depthTextureIsOwned(false)
{
	for (size_t i = 0; i < MaxColorTextureCount; ++i)
		colorTextureIds[i] = kokko::RenderTextureId();
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept :
	framebufferId(0),
	colorTextureCount(0),
	depthTextureId(0)
{
	operator=(std::move(other));
}

Framebuffer::~Framebuffer()
{
	Destroy();
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept
{
	Destroy();

	renderDevice = other.renderDevice;
	width = other.width;
	height = other.height;
	framebufferId = other.framebufferId;
	colorTextureCount = other.colorTextureCount;

	for (size_t i = 0; i < other.colorTextureCount; ++i)
	{
		colorTextureIds[i] = other.colorTextureIds[i];

		other.colorTextureIds[i] = kokko::RenderTextureId();
	}

	depthTextureId = other.depthTextureId;
	depthTextureIsOwned = other.depthTextureIsOwned;

	other.width = 0;
	other.height = 0;
	other.framebufferId = kokko::RenderFramebufferId();
	other.colorTextureCount = 0;
	other.depthTextureId = kokko::RenderTextureId();
	other.depthTextureIsOwned = false;

	return *this;
}

void Framebuffer::SetRenderDevice(RenderDevice* device)
{
	renderDevice = device;
}

bool Framebuffer::IsInitialized() const
{
	return framebufferId != 0;
}

kokko::RenderFramebufferId Framebuffer::GetFramebufferId() const
{
	return framebufferId;
}

kokko::RenderTextureId Framebuffer::GetColorTextureId(size_t index) const
{
	assert(index < colorTextureCount);

	return colorTextureIds[index];
}

kokko::RenderTextureId Framebuffer::GetDepthTextureId() const
{
	return depthTextureId;
}

int Framebuffer::GetWidth() const
{
	return width;
}

int Framebuffer::GetHeight() const
{
	return height;
}

Vec2i Framebuffer::GetSize() const
{
	return Vec2i(width, height);
}

void Framebuffer::Create(
	int width,
	int height,
	Optional<RenderTextureSizedFormat> depthFormat,
	ArrayView<RenderTextureSizedFormat> colorTextureFormats)
{
	static const RenderFramebufferAttachment colAtt[4] = {
		RenderFramebufferAttachment::Color0,
		RenderFramebufferAttachment::Color1,
		RenderFramebufferAttachment::Color2,
		RenderFramebufferAttachment::Color3,
	};

	assert(renderDevice != nullptr);
	assert(colorTextureFormats.GetCount() <= MaxColorTextureCount);
	assert(framebufferId == 0);
	
	this->width = width;
	this->height = height;

	// Create framebuffer

	renderDevice->CreateFramebuffers(1, &framebufferId);

	// Depth texture

	if (depthFormat.HasValue())
	{
		RenderTextureSizedFormat depthTextureFormat = depthFormat.GetValue();

		renderDevice->CreateTextures(RenderTextureTarget::Texture2d, 1, &depthTextureId);
		renderDevice->SetTextureStorage2D(depthTextureId, 1, depthTextureFormat, width, height);
		renderDevice->AttachFramebufferTexture(framebufferId, RenderFramebufferAttachment::Depth, depthTextureId, 0);

		depthTextureIsOwned = true;
	}

	// Color textures

	colorTextureCount = colorTextureFormats.GetCount();

	if (colorTextureCount > 0)
	{
		renderDevice->CreateTextures(RenderTextureTarget::Texture2d,
			static_cast<uint32_t>(colorTextureFormats.GetCount()), colorTextureIds);

		for (size_t i = 0, count = colorTextureFormats.GetCount(); i < count; ++i)
		{
			renderDevice->SetTextureStorage2D(colorTextureIds[i], 1, colorTextureFormats[i], width, height);
			renderDevice->AttachFramebufferTexture(framebufferId, colAtt[i], colorTextureIds[i], 0);
		}

		renderDevice->SetFramebufferDrawBuffers(static_cast<unsigned int>(colorTextureCount), colAtt);
	}
	else
	{
		RenderFramebufferAttachment noneAttachment = RenderFramebufferAttachment::None;
		renderDevice->SetFramebufferDrawBuffers(1, &noneAttachment);
	}
}

void Framebuffer::Destroy()
{
	assert(renderDevice != nullptr);

	if (framebufferId != 0)
	{
		renderDevice->DestroyFramebuffers(1, &framebufferId);
		framebufferId = kokko::RenderFramebufferId();

		width = 0;
		height = 0;

		if (colorTextureCount > 0)
		{
			renderDevice->DestroyTextures(static_cast<unsigned int>(colorTextureCount), colorTextureIds);

			for (size_t i = 0; i < colorTextureCount; ++i)
				colorTextureIds[i] = kokko::RenderTextureId();

			colorTextureCount = 0;
		}

		if (depthTextureId != 0)
		{
			if (depthTextureIsOwned)
				renderDevice->DestroyTextures(1, &depthTextureId);

			depthTextureId = kokko::RenderTextureId();
			depthTextureIsOwned = false;
		}
	}
}

void Framebuffer::AttachExternalDepthTexture(kokko::RenderTextureId textureId)
{
	assert(depthTextureId == 0);

	depthTextureId = textureId;
	depthTextureIsOwned = false;

	renderDevice->AttachFramebufferTexture(framebufferId, RenderFramebufferAttachment::Depth, depthTextureId, 0);
}

void Framebuffer::SetDebugLabel(kokko::ConstStringView label)
{
	renderDevice->SetObjectLabel(RenderObjectType::Framebuffer, framebufferId.i, label);
}
