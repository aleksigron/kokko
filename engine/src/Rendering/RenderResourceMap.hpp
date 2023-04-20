#pragma once

#include <cstdint>

#include "Core/HashMap.hpp"

#include "Rendering/RenderResourceId.hpp"

class Allocator;

namespace kokko
{

namespace render
{

template <typename ResourceId>
class ResourceIdMap
{
public:
	explicit ResourceIdMap(Allocator* allocator);

	void CreateIds(uint32_t count, ResourceId* idsOut);
	void DeleteIds(uint32_t count, const ResourceId* ids);

	void SetDeviceId(ResourceId bufferId, uint32_t bufferDeviceId);
	uint32_t GetDeviceId(ResourceId bufferId) const;

private:
	HashMap<uint32_t, uint32_t> idMap;
	uint32_t lastUsedId;
};

struct ResourceMap
{
	explicit ResourceMap(Allocator* allocator);

	ResourceIdMap<render::BufferId> bufferIds;
	ResourceIdMap<render::FramebufferId> framebufferIds;
	ResourceIdMap<render::SamplerId> samplerIds;
	ResourceIdMap<render::ShaderId> shaderIds;
	ResourceIdMap<render::TextureId> textureIds;
	ResourceIdMap<render::VertexArrayId> vertexArrayIds;
};

}
}
