#pragma once

#include <cstdint>

namespace kokko
{

enum class RenderDebugFeatureFlag : uint32_t
{
	None = 0,
	DrawBounds = 1 << 0,
	DrawNormals = 1 << 1,
	DrawTerrainTiles = 1 << 2,
	ExperimentalTerrainShadows = 1 << 3
};

}
