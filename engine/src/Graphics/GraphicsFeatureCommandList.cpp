#include "Graphics/GraphicsFeatureCommandList.hpp"

#include "Rendering/RendererCommandList.hpp"

namespace kokko
{

GraphicsFeatureCommandList::GraphicsFeatureCommandList(
	RendererCommandList& list,
	uint64_t fullscreenViewport,
	uint64_t featureIndex) :
	list(list),
	fullscreenViewportIndex(fullscreenViewport),
	featureIndex(featureIndex)
{
}

void GraphicsFeatureCommandList::AddToStartOfFrame(uint16_t object)
{
	list.AddDrawWithCallback(0, RenderPassType::Setup, 0.0f, featureIndex, object);
}

void GraphicsFeatureCommandList::AddToFullscreenViewport(RenderPassType pass, float depth, uint16_t object)
{
	list.AddDrawWithCallback(fullscreenViewportIndex, pass, depth, featureIndex, object);
}

void GraphicsFeatureCommandList::AddToFullscreenViewportWithOrder(RenderPassType pass, uint64_t order, uint16_t object)
{
	list.AddGraphicsFeatureWithOrder(fullscreenViewportIndex, pass, order, featureIndex, object);
}

}