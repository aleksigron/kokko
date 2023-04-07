#pragma once

#include <cstddef>

#include "Core/ArrayView.hpp"
#include "Core/Optional.hpp"
#include "Core/StringView.hpp"

#include "Math/Vec2.hpp"

#include "Rendering/RenderResourceId.hpp"

class RenderDevice;

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

	kokko::RenderFramebufferId GetFramebufferId() const;
	kokko::RenderTextureId GetColorTextureId(size_t index) const;
	kokko::RenderTextureId GetDepthTextureId() const;

	int GetWidth() const;
	int GetHeight() const;
	Vec2i GetSize() const;

	void Create(int width, int height, Optional<RenderTextureSizedFormat> depthFormat,
		ArrayView<RenderTextureSizedFormat> colorTextureFormats);
	void Destroy();

	void AttachExternalDepthTexture(kokko::RenderTextureId textureId);

	void SetDebugLabel(kokko::ConstStringView label);

private:
	RenderDevice* renderDevice;

	int width;
	int height;

	kokko::RenderFramebufferId framebufferId;

	size_t colorTextureCount;
	kokko::RenderTextureId colorTextureIds[MaxColorTextureCount];

	kokko::RenderTextureId depthTextureId;
	bool depthTextureIsOwned;
};
