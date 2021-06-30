#pragma once

#include <cstdint>

#include "Core/ArrayView.hpp"
#include "Core/Optional.hpp"

#include "Rendering/RenderDeviceEnums.hpp"

class RenderDevice;

class Framebuffer
{
public:
	static const size_t MaxColorTextureCount = 4;

	Framebuffer();
	Framebuffer(const Framebuffer&) = delete;
	Framebuffer(Framebuffer&& other) noexcept;
	~Framebuffer();

	Framebuffer& operator=(const Framebuffer&) = delete;
	Framebuffer& operator=(Framebuffer&& other) noexcept;

	void SetRenderDevice(RenderDevice* device);

	bool IsInitialized() const;

	unsigned int GetFramebufferId() const;
	unsigned int GetColorTextureId(size_t index) const;
	unsigned int GetDepthTextureId() const;

	int GetWidth() const;
	int GetHeight() const;

	void Create(int width, int height, Optional<RenderTextureSizedFormat> depthFormat,
		ArrayView<RenderTextureSizedFormat> colorTextureFormats);
	void Destroy();

private:
	void CreateTexture(RenderTextureSizedFormat format, int width, int height);

	RenderDevice* renderDevice;

	int width;
	int height;

	unsigned int framebufferId;

	size_t colorTextureCount;
	RenderTextureSizedFormat colorTextureFormats[MaxColorTextureCount];
	unsigned int colorTextureIds[MaxColorTextureCount];

	RenderTextureSizedFormat depthTextureFormat;
	unsigned int depthTextureId;
};
