#pragma once

#include <cstddef>
#include <cstdint>

class Allocator;
class RenderDevice;

namespace kokko
{

struct TerrainTile
{
	static constexpr int Resolution = 63;
	static constexpr int ResolutionWithBorder = Resolution + 1;

	uint16_t heightData[ResolutionWithBorder * ResolutionWithBorder];
};

class TerrainQuadTree
{
public:
	TerrainQuadTree();

	void CreateResources(Allocator* allocator, RenderDevice* renderDevice, int levels);
	void DestroyResources(Allocator* allocator, RenderDevice* renderDevice);

	int GetLevelCount() const;

	const TerrainTile* GetTile(int level, int x, int y);
	unsigned int GetTileHeightTexture(int level, int x, int y);

	static int GetTilesPerDimension(int level);
	static int GetTileIndex(int level, int x, int y);
	static int GetTileCountForLevelCount(int levelCount);
	static float GetTileScale(int level);

	static void CreateTileTestData(TerrainTile& tile, int tileX, int tileY, float tileScale);

private:

	static uint16_t TestData(float x, float y);

	TerrainTile* tiles;
	uint32_t* tileTextureIds;

	int treeLevels;
	int tileCount;
};

}
