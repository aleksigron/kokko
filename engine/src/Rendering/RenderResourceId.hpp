#pragma once

#include <cstdint>

#define DECLARE_RENDER_RESOURCE_ID(name, defaultName) struct name\
{\
	uint32_t i;\
	static const name defaultName;\
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

DECLARE_RENDER_RESOURCE_ID(BufferId, Null);
DECLARE_RENDER_RESOURCE_ID(FramebufferId, Default);
DECLARE_RENDER_RESOURCE_ID(SamplerId, Null);
DECLARE_RENDER_RESOURCE_ID(ShaderId, Null);
DECLARE_RENDER_RESOURCE_ID(TextureId, Null);
DECLARE_RENDER_RESOURCE_ID(VertexArrayId, Null);

}
}

#undef DECLARE_RENDER_RESOURCE_ID
