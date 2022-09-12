#pragma once

#include "Math/Vec2.hpp"

#include "Rendering/RenderTypes.hpp"

class Allocator;
class RenderDevice;

struct RenderTarget
{
	unsigned int id;
	Vec2i size;

	RenderTextureSizedFormat colorFormat;
	unsigned int colorTexture;

	unsigned int framebuffer;
};

class RenderTargetContainer
{
private:
	static const size_t MaxRenderTargetCount = 32;

	struct TargetInfo
	{
		RenderTarget target;
		bool inUse;
	};

	Allocator* allocator;
	RenderDevice* renderDevice;

	TargetInfo renderTargets[MaxRenderTargetCount];
	size_t renderTargetCount;

public:
	RenderTargetContainer(Allocator* allocator, RenderDevice* renderDevice);
	~RenderTargetContainer();

	RenderTarget AcquireRenderTarget(Vec2i size, RenderTextureSizedFormat format);
	void ReleaseRenderTarget(unsigned int renderTargetId);

	bool ConfirmAllTargetsAreUnused();

	void DestroyAllRenderTargets();
};
