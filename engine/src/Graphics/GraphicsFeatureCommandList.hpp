#pragma once

#include <cstdint>

class Renderer;

struct RenderCommandList;

enum class RenderPass;

namespace kokko
{

class GraphicsFeatureCommandList
{
public:
	GraphicsFeatureCommandList(RenderCommandList& list, uint64_t fullscreenViewport, uint64_t featureIndex);

	void AddToFullscreenViewport(RenderPass pass, float depth, uint16_t object);

private:
	RenderCommandList& list;
	uint64_t fullscreenViewportIndex;
	uint64_t featureIndex;

	friend class Renderer;
};

}
