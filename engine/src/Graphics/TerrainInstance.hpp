#pragma once

#include <cstdint>

#include "Math/Vec2.hpp"

#include "Resources/MeshId.hpp"

struct TerrainInstance
{
	kokko::MeshId meshId;

	float terrainSize;
	int terrainResolution;
	
	Vec2f textureScale;

	float minHeight;
	float maxHeight;
	uint16_t* heightData;

	unsigned int vertexArrayId;
	unsigned int uniformBufferId;
	unsigned int textureId;

	TerrainInstance();
};
