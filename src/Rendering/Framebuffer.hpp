#pragma once

#include <cstddef>

#include "Core/ArrayView.hpp"
#include "Core/Optional.hpp"

#include "Math/Vec2.hpp"

class RenderDevice;

struct StringRef;

enum class RenderTextureSizedFormat;
enum class RenderTextureCompareMode;
enum class RenderDepthCompareFunc;

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
	Vec2i GetSize() const;

	void Create(int width, int height, Optional<RenderTextureSizedFormat> depthFormat,
		ArrayView<RenderTextureSizedFormat> colorTextureFormats);
	void Destroy();

	void AttachExternalDepthTexture(unsigned int textureId);
	void SetDepthTextureCompare(RenderTextureCompareMode mode, RenderDepthCompareFunc func);

	void SetDebugLabel(StringRef label);

private:
	void CreateTexture(RenderTextureSizedFormat format, int width, int height);

	RenderDevice* renderDevice;

	int width;
	int height;

	unsigned int framebufferId;

	size_t colorTextureCount;
	unsigned int colorTextureIds[MaxColorTextureCount];

	unsigned int depthTextureId;
	bool depthTextureIsOwned;
};
