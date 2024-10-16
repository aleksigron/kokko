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
	GraphicsFeatureCommandList(RendererCommandList& list, uint64_t featureIndex);

	void AddToStartOfFrame(uint16_t object);
	void AddToViewport(uint32_t viewportIndex, RenderPassType pass, float depth, uint16_t object);
	void AddToViewportWithOrder(uint32_t viewportIndex, RenderPassType pass, uint64_t order, uint16_t object);

private:
	RendererCommandList& list;
	uint64_t featureIndex;

	friend class Renderer;
};

} // namespace kokko
