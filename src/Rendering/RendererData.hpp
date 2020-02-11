#pragma once

#include "Resources/MaterialData.hpp"
#include "Rendering/TransparencyType.hpp"

struct RenderObjectId
{
	unsigned int i;

	bool IsNull() const { return i == 0; }
};

struct RenderOrderData
{
	MaterialId material;
	TransparencyType transparency;
};
