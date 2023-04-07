#include "Rendering/RenderResourceMap.hpp"

#include <cassert>

namespace kokko
{
namespace render
{

template <typename ResourceId>
ResourceIdMap<ResourceId>::ResourceIdMap(Allocator* allocator) :
	idMap(allocator),
	lastUsedId(0)
{
}

template <typename ResourceId>
void ResourceIdMap<ResourceId>::CreateIds(uint32_t count, ResourceId* idsOut)
{
	for (uint32_t i = 0; i < count; ++i)
	{
		lastUsedId += 1;

		// TODO: Implement ID recycling
		assert(lastUsedId != UINT32_MAX);

		auto pair = idMap.Insert(lastUsedId);
		pair->second = 0;

		idsOut[i] = ResourceId(lastUsedId);
	}
}

template <typename ResourceId>
void ResourceIdMap<ResourceId>::DeleteIds(uint32_t count, const ResourceId* ids)
{
	for (uint32_t i = 0; i < count; ++i)
	{
		auto pair = idMap.Lookup(ids[i].i);
		if (pair != nullptr)
			idMap.Remove(pair);
	}
}

template <typename ResourceId>
void ResourceIdMap<ResourceId>::SetDeviceId(ResourceId resourceId, uint32_t deviceId)
{
	auto pair = idMap.Lookup(resourceId.i);
	if (pair != nullptr)
		pair->second = deviceId;
}

template <typename ResourceId>
uint32_t ResourceIdMap<ResourceId>::GetDeviceId(ResourceId resourceId) const
{
	auto pair = idMap.Lookup(resourceId.i);
	if (pair != nullptr)
		return pair->second;

	return 0;
}

// Explicitly instantiate specific types
template class ResourceIdMap<RenderBufferId>;
template class ResourceIdMap<RenderFramebufferId>;
template class ResourceIdMap<RenderSamplerId>;
template class ResourceIdMap<RenderShaderId>;
template class ResourceIdMap<RenderTextureId>;
template class ResourceIdMap<RenderVertexArrayId>;

ResourceMap::ResourceMap(Allocator* allocator) :
	bufferIds(allocator),
	framebufferIds(allocator),
	samplerIds(allocator),
	shaderIds(allocator),
	textureIds(allocator),
	vertexArrayIds(allocator)
{
}

} // namespace render
} // namespace kokko
