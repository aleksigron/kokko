#pragma once

#include <cstddef>
#include <cstdint>

#include "Core/Array.hpp"

#include "Math/Vec3.hpp"

class Allocator;

struct FrustumPlanes;
struct Mat4x4f;
struct CameraParameters;
struct TerrainTileId;

namespace kokko
{

class RenderDebugSettings;

struct TerrainParameters;

namespace render
{
struct TextureId;
class Device;
}

struct TerrainTile
{
	static constexpr int Resolution = 31;
	static constexpr int ResolutionWithBorder = Resolution + 1;

	uint16_t heightData[ResolutionWithBorder * ResolutionWithBorder];
};

struct TerrainQuadTreeNode
{
	uint32_t x;
	uint32_t y;
	uint16_t children[4];
	uint8_t numChildren;
	uint8_t level;
};

class TerrainQuadTree
{
public:
	TerrainQuadTree(Allocator* allocator);

	void CreateResources(kokko::render::Device* renderDevice, uint8_t levels,
		const TerrainParameters& params);
	void DestroyResources(kokko::render::Device* renderDevice);

	void GetTilesToRender(const FrustumPlanes& frustum, const Vec3f& cameraPos,
		const RenderDebugSettings& renderDebug, Array<TerrainTileId>& resultOut);

	int GetLevelCount() const;

	float GetSize() const { return terrainWidth; }
	void SetSize(float size) { terrainWidth = size; }

	float GetBottom() const { return terrainBottom; }
	void SetBottom(float bottom) { terrainBottom = bottom; }

	float GetHeight() const { return terrainHeight; }
	void SetHeight(float height) { terrainHeight = height; }

	const TerrainTile* GetTile(uint8_t level, int x, int y);
	render::TextureId GetTileHeightTexture(uint8_t level, int x, int y);

	static int GetTilesPerDimension(uint8_t level);
	static int GetTileIndex(uint8_t level, int x, int y);
	static int GetTileCountForLevelCount(uint8_t levelCount);
	static float GetTileScale(uint8_t level);

private:
	struct GetRenderTilesParams
	{
		const FrustumPlanes& frustum;
		const Vec3f& cameraPos;
		const RenderDebugSettings& renderDebug;
		Array<TerrainTileId>& resultOut;
	};

	// Returns inserted node index (points to nodes array), or -1 if no insertion
	int RenderTile(const TerrainTileId& id, GetRenderTilesParams& params);

	static void CreateTileTestData(TerrainTile& tile, int tileX, int tileY, float tileScale);

	static uint16_t TestData(float x, float y);

	Allocator* allocator;
	Array<TerrainQuadTreeNode> nodes;
	TerrainTile* tiles;
	render::TextureId* tileTextureIds;

	uint8_t treeLevels;
	int tileCount;
	float terrainWidth;
	float terrainBottom;
	float terrainHeight;
};

}
