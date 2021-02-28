#pragma once

#include "Math/Vec2.hpp"

#include "Rendering/RenderDeviceEnums.hpp"

class Allocator;
class RenderDevice;

struct RenderTarget
{
	Vec2i size;

	RenderTextureSizedFormat colorFormat;
	unsigned int colorTexture;

	unsigned int framebuffer;

	bool inUse;
};

class RenderTargetContainer
{
private:
	static const size_t MaxRenderTargetCount = 32;

	Allocator* allocator;
	RenderDevice* renderDevice;

	RenderTarget* renderTargets;
	size_t renderTargetCount;

public:
	RenderTargetContainer(Allocator* allocator, RenderDevice* renderDevice);
	~RenderTargetContainer();

	RenderTarget* AcquireRenderTarget(Vec2i size, RenderTextureSizedFormat format);
	void ReleaseRenderTarget(RenderTarget* target);
};
