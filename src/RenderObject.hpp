#pragma once

#include <cstdint>

#include "ObjectId.hpp"

struct RenderObject
{
	ObjectId id;

	ObjectId mesh;
	uint32_t materialId;
	unsigned int sceneObjectId;
};