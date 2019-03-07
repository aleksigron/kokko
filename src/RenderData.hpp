#pragma once

#include <cstdint>

#include "Entity.hpp"
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

enum class TransparencyType
{
	Opaque,
	AlphaTest,
	TransparentMix,
	TransparentAdd,
	TransparentSub
};
