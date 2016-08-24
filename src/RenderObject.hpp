#pragma once

#include <cstdint>

#include "ObjectId.hpp"

struct RenderObject
{
	ObjectId mesh;
	uint32_t materialId;
	unsigned int sceneObjectId;
};
