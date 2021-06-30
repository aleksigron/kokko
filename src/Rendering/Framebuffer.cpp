#include "Rendering/Framebuffer.hpp"

#include <cassert>
#include <utility>

#include "Rendering/RenderDevice.hpp"

Framebuffer::Framebuffer() :
	renderDevice(nullptr),
	width(0),
	height(0),
	framebufferId(0),
	colorTextureCount(0)
{
	for (size_t i = 0; i < MaxColorTextureCount; ++i)
	{
		colorTextureFormats[i] = RenderTextureSizedFormat::RGB8;
		colorTextureIds[i] = 0;
	}

	depthTextureFormat = RenderTextureSizedFormat::D24;
	depthTextureId = 0;
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
		colorTextureFormats[i] = other.colorTextureFormats[i];
		colorTextureIds[i] = other.colorTextureIds[i];

		other.colorTextureIds[i] = 0;
	}

	depthTextureFormat = other.depthTextureFormat;
	depthTextureId = other.depthTextureId;

	other.width = 0;
	other.height = 0;
	other.framebufferId = 0;
	other.colorTextureCount = 0;
	other.depthTextureId = 0;

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

unsigned int Framebuffer::GetFramebufferId() const
{
	return framebufferId;
}

unsigned int Framebuffer::GetColorTextureId(size_t index) const
{
	assert(index < colorTextureCount);

	return colorTextureIds[index];
}

unsigned int Framebuffer::GetDepthTextureId() const
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


	// Create and bind framebuffer

	renderDevice->CreateFramebuffers(1, &framebufferId);
	renderDevice->BindFramebuffer(RenderFramebufferTarget::Framebuffer, framebufferId);

	// Depth texture

	if (depthFormat.HasValue())
	{
		depthTextureFormat = depthFormat.GetValue();

		renderDevice->CreateTextures(1, &depthTextureId);
		renderDevice->BindTexture(RenderTextureTarget::Texture2d, depthTextureId);

		CreateTexture(depthTextureFormat, width, height);

		RenderCommandData::AttachFramebufferTexture2D attachTexture{
			RenderFramebufferTarget::Framebuffer, RenderFramebufferAttachment::Depth,
			RenderTextureTarget::Texture2d, depthTextureId, 0
		};
		renderDevice->AttachFramebufferTexture2D(&attachTexture);
	}

	// Color textures

	colorTextureCount = colorTextureFormats.GetCount();

	renderDevice->CreateTextures(static_cast<unsigned int>(colorTextureFormats.GetCount()), colorTextureIds);

	for (size_t i = 0, count = colorTextureFormats.GetCount(); i < count; ++i)
	{
		this->colorTextureFormats[i] = colorTextureFormats[i];
		unsigned int textureId = colorTextureIds[i];

		renderDevice->BindTexture(RenderTextureTarget::Texture2d, textureId);

		CreateTexture(colorTextureFormats[i], width, height);

		RenderCommandData::AttachFramebufferTexture2D attachTexture{
			RenderFramebufferTarget::Framebuffer, colAtt[i], RenderTextureTarget::Texture2d, textureId, 0
		};
		renderDevice->AttachFramebufferTexture2D(&attachTexture);
	}
}

void Framebuffer::Destroy()
{
	assert(renderDevice != nullptr);

	if (framebufferId != 0)
	{
		if (colorTextureCount > 0)
		{
			renderDevice->DestroyTextures(static_cast<unsigned int>(colorTextureCount), colorTextureIds);

			for (size_t i = 0; i < colorTextureCount; ++i)
				colorTextureIds[i] = 0;

			colorTextureCount = 0;
		}

		if (depthTextureId != 0)
		{
			renderDevice->DestroyTextures(1, &depthTextureId);
			depthTextureId = 0;
		}

		renderDevice->DestroyFramebuffers(1, &framebufferId);
		framebufferId = 0;
	}
}

void Framebuffer::CreateTexture(RenderTextureSizedFormat format, int width, int height)
{
	RenderCommandData::SetTextureStorage2D textureStorage{ RenderTextureTarget::Texture2d, 1, format, width, height };
	renderDevice->SetTextureStorage2D(&textureStorage);

	renderDevice->SetTextureMinFilter(RenderTextureTarget::Texture2d, RenderTextureFilterMode::Nearest);
	renderDevice->SetTextureMagFilter(RenderTextureTarget::Texture2d, RenderTextureFilterMode::Nearest);
	renderDevice->SetTextureWrapModeU(RenderTextureTarget::Texture2d, RenderTextureWrapMode::ClampToEdge);
	renderDevice->SetTextureWrapModeV(RenderTextureTarget::Texture2d, RenderTextureWrapMode::ClampToEdge);
	renderDevice->SetTextureWrapModeW(RenderTextureTarget::Texture2d, RenderTextureWrapMode::ClampToEdge);
}
