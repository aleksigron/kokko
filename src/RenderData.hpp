#pragma once

#include <cstdint>

#include "ObjectId.hpp"
#include "SceneLayer.hpp"

struct RenderCommand
{
	uint64_t orderKey;
	unsigned int renderObjectIndex;

	RenderCommand(uint64_t controlCommand) : orderKey(controlCommand)
	{
	}

	RenderCommand(uint64_t drawCommand, unsigned int renderObjectIndex) :
		orderKey(drawCommand),
		renderObjectIndex(renderObjectIndex)
	{
	}

	bool operator<(const RenderCommand& rhs)
	{
		return this->orderKey < rhs.orderKey;
	}
};

struct RenderObject
{
	unsigned int meshId;
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
