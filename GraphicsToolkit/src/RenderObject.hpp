#pragma once

#include "ObjectId.hpp"

struct RenderObject
{
	ObjectId id;

	ObjectId material;
	ObjectId mesh;
	unsigned int sceneObjectId;
};