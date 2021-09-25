#pragma once

#include "Resources/MaterialData.hpp"
#include "Rendering/TransparencyType.hpp"

struct RenderObjectId
{
	unsigned int i;

	bool operator==(RenderObjectId other) { return i == other.i; }
	bool operator!=(RenderObjectId other) { return !operator==(other); }

	static const RenderObjectId Null;
};

struct RenderOrderData
{
	MaterialId material;
	TransparencyType transparency;
};
