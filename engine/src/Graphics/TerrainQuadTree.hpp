#pragma once

#include <cstddef>
#include <cstdint>

class Allocator;
class RenderDevice;

namespace kokko
{

struct TerrainTile
{
	static const int Resolution = 64;

	unsigned int textureId;
	uint16_t heightData[Resolution * Resolution];
};

class TerrainQuadTree
{
public:
	TerrainQuadTree(Allocator* allocator, RenderDevice* renderDevice);
	~TerrainQuadTree();

	void Initialize(int levels);

	static int GetTilesPerDimension(int level);
	static int GetTileIndex(int level, int x, int y);
	static int GetTileCountForLevelCount(int levelCount);

private:
	Allocator* allocator;
	RenderDevice* renderDevice;

	TerrainTile* tiles;
	uint32_t* tileTextureIds;

	int treeLevels;
	size_t tileCount;
};

}
