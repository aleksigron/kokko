#pragma once

#include <cstddef>
#include <cstdint>

class Allocator;
class RenderDevice;

namespace kokko
{

struct TerrainTile
{
	static constexpr int Resolution = 64;

	uint16_t heightData[Resolution * Resolution];
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

private:
	static void CreateTileTestData(TerrainTile& tile, float tileScale);

	static uint16_t TestData(float x, float y);

	TerrainTile* tiles;
	uint32_t* tileTextureIds;

	int treeLevels;
	int tileCount;
};

}
