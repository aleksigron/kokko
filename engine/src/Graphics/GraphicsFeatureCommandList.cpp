#include "Graphics/GraphicsFeatureCommandList.hpp"

#include "Rendering/RenderCommandList.hpp"

namespace kokko
{

GraphicsFeatureCommandList::GraphicsFeatureCommandList(
	RenderCommandList& list,
	uint64_t fullscreenViewport,
	uint64_t featureIndex) :
	list(list),
	fullscreenViewportIndex(fullscreenViewport),
	featureIndex(featureIndex)
{
}

void GraphicsFeatureCommandList::AddToFullscreenViewport(RenderPass pass, float depth, uint16_t object)
{
	list.AddDrawWithCallback(fullscreenViewportIndex, pass, depth, featureIndex, true, object);
}

}