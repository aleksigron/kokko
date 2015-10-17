#pragma once

#include "ObjectId.h"

struct RenderObject
{
	ObjectId id;

	ObjectId material;
	ObjectId mesh;
	unsigned int sceneObjectId;
};