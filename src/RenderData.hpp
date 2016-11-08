#pragma once

#include <cstdint>

#include "ObjectId.hpp"
#include "SceneLayer.hpp"

struct DrawCall
{
	uint64_t orderKey;
	unsigned int renderObjectIndex;
};

struct RenderObject
{
	ObjectId meshId;
	unsigned int materialId;
	unsigned int sceneObjectId;
	SceneLayer layer;
};

enum class TransparencyType
{
	Opaque,
	AlphaTest,
	TransparentMix,
	TransparentAdd,
	TransparentSub
};
