#pragma once

#include <cstdint>

#include "ObjectId.hpp"

struct RenderObject
{
	ObjectId meshId;
	unsigned int materialId;
	unsigned int sceneObjectId;
	unsigned int layer;
};
