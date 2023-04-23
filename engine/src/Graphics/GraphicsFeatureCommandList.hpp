#pragma once

#include <cstdint>

namespace kokko
{

class Renderer;

struct RendererCommandList;

enum class RenderPassType;

class GraphicsFeatureCommandList
{
public:
	GraphicsFeatureCommandList(RendererCommandList& list, uint64_t fullscreenViewport, uint64_t featureIndex);

	void AddToStartOfFrame(uint16_t object);
	void AddToFullscreenViewport(RenderPassType pass, float depth, uint16_t object);
	void AddToFullscreenViewportWithOrder(RenderPassType pass, uint64_t order, uint16_t object);

private:
	RendererCommandList& list;
	uint64_t fullscreenViewportIndex;
	uint64_t featureIndex;

	friend class Renderer;
};

} // namespace kokko
