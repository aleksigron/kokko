#pragma once

#include <cstddef>
#include <cstdint>

#include "Core/Array.hpp"

class Allocator;
class RenderDevice;

struct Mat4x4f;
struct CameraParameters;
struct TerrainTileId;

namespace kokko
{

struct TerrainTile
{
	static constexpr int Resolution = 31;
	static constexpr int ResolutionWithBorder = Resolution + 1;

	uint16_t heightData[ResolutionWithBorder * ResolutionWithBorder];
};

class TerrainQuadTree
{
public:
	TerrainQuadTree();

	void CreateResources(Allocator* allocator, RenderDevice* renderDevice, int levels, float size);
	void DestroyResources(Allocator* allocator, RenderDevice* renderDevice);

	void GetTilesToRender(const Mat4x4f& viewProj, Array<TerrainTileId>& resultOut);

	int GetLevelCount() const;

	const TerrainTile* GetTile(int level, int x, int y);
	unsigned int GetTileHeightTexture(int level, int x, int y);

	static int GetTilesPerDimension(int level);
	static int GetTileIndex(int level, int x, int y);
	static int GetTileCountForLevelCount(int levelCount);
	static float GetTileScale(int level);

private:
	void RenderTile(const TerrainTileId& id, const Mat4x4f& vp, Array<TerrainTileId>& resultOut);

	static void CreateTileTestData(TerrainTile& tile, int tileX, int tileY, float tileScale);

	static uint16_t TestData(float x, float y);

	TerrainTile* tiles;
	uint32_t* tileTextureIds;

	int treeLevels;
	int tileCount;
	float terrainSize;
};

}
