#pragma once

#include <cstdint>

#define DEFINE_RENDER_RESOURCE_ID(name) struct name\
{\
	uint32_t i;\
	name() : i(0) {}\
	explicit name(uint32_t v) : i(v) {}\
	bool operator==(const name& other) const { return i == other.i; }\
	bool operator!=(const name& other) const { return !operator==(other); }\
	bool operator==(const uint32_t& other) const { return i == other; }\
	bool operator!=(const uint32_t& other) const { return !operator==(other); }\
}

namespace kokko
{
namespace render
{

DEFINE_RENDER_RESOURCE_ID(BufferId);
DEFINE_RENDER_RESOURCE_ID(FramebufferId);
DEFINE_RENDER_RESOURCE_ID(SamplerId);
DEFINE_RENDER_RESOURCE_ID(ShaderId);
DEFINE_RENDER_RESOURCE_ID(TextureId);
DEFINE_RENDER_RESOURCE_ID(VertexArrayId);

}
}

#undef DEFINE_RENDER_RESOURCE_ID
