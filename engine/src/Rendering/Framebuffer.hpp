#pragma once

#include <cstddef>

#include "Core/ArrayView.hpp"
#include "Core/Optional.hpp"
#include "Core/StringView.hpp"

#include "Math/Vec2.hpp"

#include "Rendering/RenderResourceId.hpp"

namespace kokko
{

enum class RenderTextureSizedFormat;
enum class RenderTextureCompareMode;
enum class RenderDepthCompareFunc;

namespace render
{

class Device;

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

	void SetRenderDevice(Device* device);

	bool IsInitialized() const;

	FramebufferId GetFramebufferId() const;
	TextureId GetColorTextureId(size_t index) const;
	TextureId GetDepthTextureId() const;

	int GetWidth() const;
	int GetHeight() const;
	Vec2i GetSize() const;

	void Create(int width, int height, Optional<RenderTextureSizedFormat> depthFormat,
		ArrayView<RenderTextureSizedFormat> colorTextureFormats);
	void Destroy();

	void AttachExternalDepthTexture(TextureId textureId);

	void SetDebugLabel(ConstStringView label);

private:
	Device* renderDevice;

	int width;
	int height;

	FramebufferId framebufferId;

	size_t colorTextureCount;
	TextureId colorTextureIds[MaxColorTextureCount];

	TextureId depthTextureId;
	bool depthTextureIsOwned;
};

} // namespace render
} // namespace kokko
