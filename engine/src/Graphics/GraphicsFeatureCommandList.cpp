#include "Graphics/GraphicsFeatureCommandList.hpp"

#include "Rendering/RendererCommandList.hpp"

namespace kokko
{

GraphicsFeatureCommandList::GraphicsFeatureCommandList(RendererCommandList& list, uint64_t featureIndex) :
	list(list),
	featureIndex(featureIndex)
{
}

void GraphicsFeatureCommandList::AddToStartOfFrame(uint16_t object)
{
	list.AddDrawWithCallback(0, RenderPassType::Setup, 0.0f, featureIndex, object);
}

void GraphicsFeatureCommandList::AddToViewport(uint32_t viewportIndex, RenderPassType pass, float depth, uint16_t object)
{
	list.AddDrawWithCallback(viewportIndex, pass, depth, featureIndex, object);
}

void GraphicsFeatureCommandList::AddToViewportWithOrder(uint32_t viewportIndex, RenderPassType pass, uint64_t order, uint16_t object)
{
	list.AddGraphicsFeatureWithOrder(viewportIndex, pass, order, featureIndex, object);
}

}