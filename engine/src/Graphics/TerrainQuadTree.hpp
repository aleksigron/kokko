#pragma once

#include <cstddef>
#include <cstdint>

#include "Core/Array.hpp"

class Allocator;
class RenderDevice;

struct FrustumPlanes;
struct Mat4x4f;
struct CameraParameters;
struct TerrainTileId;

namespace kokko
{

struct RenderTextureId;
struct TerrainParameters;

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

	void CreateResources(Allocator* allocator, RenderDevice* renderDevice, int levels,
		const TerrainParameters& params);
	void DestroyResources(Allocator* allocator, RenderDevice* renderDevice);

	void GetTilesToRender(
		const FrustumPlanes& frustum, const Mat4x4f& viewProj, Array<TerrainTileId>& resultOut);

	int GetLevelCount() const;

	float GetSize() const { return terrainWidth; }
	void SetSize(float size) { terrainWidth = size; }

	float GetBottom() const { return terrainBottom; }
	void SetBottom(float bottom) { terrainBottom = bottom; }

	float GetHeight() const { return terrainHeight; }
	void SetHeight(float height) { terrainHeight = height; }

	const TerrainTile* GetTile(int level, int x, int y);
	RenderTextureId GetTileHeightTexture(int level, int x, int y);

	static int GetTilesPerDimension(int level);
	static int GetTileIndex(int level, int x, int y);
	static int GetTileCountForLevelCount(int levelCount);
	static float GetTileScale(int level);

private:
	void RenderTile(
		const TerrainTileId& id,
		const FrustumPlanes& frustum,
		const Mat4x4f& vp,
		Array<TerrainTileId>& resultOut);

	static void CreateTileTestData(TerrainTile& tile, int tileX, int tileY, float tileScale);

	static uint16_t TestData(float x, float y);

	TerrainTile* tiles;
	RenderTextureId* tileTextureIds;

	int treeLevels;
	int tileCount;
	float terrainWidth;
	float terrainBottom;
	float terrainHeight;
};

}
