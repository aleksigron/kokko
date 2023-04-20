#pragma once

#include "Math/Vec2.hpp"

#include "Rendering/RenderTypes.hpp"
#include "Rendering/RenderResourceId.hpp"

class Allocator;

namespace kokko
{
namespace render
{
class Device;
}
}

struct RenderTarget
{
	unsigned int id;
	Vec2i size;

	RenderTextureSizedFormat colorFormat;
	kokko::render::TextureId colorTexture;

	kokko::render::FramebufferId framebuffer;
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
	kokko::render::Device* renderDevice;

	TargetInfo renderTargets[MaxRenderTargetCount];
	size_t renderTargetCount;

public:
	RenderTargetContainer(Allocator* allocator, kokko::render::Device* renderDevice);
	~RenderTargetContainer();

	RenderTarget AcquireRenderTarget(Vec2i size, RenderTextureSizedFormat format);
	void ReleaseRenderTarget(unsigned int renderTargetId);

	bool ConfirmAllTargetsAreUnused();

	void DestroyAllRenderTargets();
};
