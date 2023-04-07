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

DEFINE_RENDER_RESOURCE_ID(RenderBufferId);
DEFINE_RENDER_RESOURCE_ID(RenderFramebufferId);
DEFINE_RENDER_RESOURCE_ID(RenderSamplerId);
DEFINE_RENDER_RESOURCE_ID(RenderShaderId);
DEFINE_RENDER_RESOURCE_ID(RenderTextureId);
DEFINE_RENDER_RESOURCE_ID(RenderVertexArrayId);

}

#undef DEFINE_RENDER_RESOURCE_ID
