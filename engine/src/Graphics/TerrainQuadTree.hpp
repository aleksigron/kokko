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
	static constexpr int QuadsPerSide = 32;
	static constexpr int VerticesPerSide = QuadsPerSide + 1;
	static constexpr int TexelsPerSide = VerticesPerSide + 2;
	static constexpr int TexelsPerTextureRow = TexelsPerSide + 1; // Texel data rows need a stride of 4 

	uint16_t heightData[TexelsPerTextureRow * TexelsPerSide];
};

struct QuadTreeNodeId
{
	uint32_t x;
	uint32_t y;
	uint8_t level;

	bool operator==(const QuadTreeNodeId& other) const
	{
		return x == other.x && y == other.y && level == other.level;
	}

	bool operator<(const QuadTreeNodeId& other) const
	{
		if (level < other.level) return true;
		if (level > other.level) return false;
		if (x < other.level) return true;
		if (x > other.level) return false;
		return y < other.y;
	}
};

struct TerrainQuadTreeNode
{
	QuadTreeNodeId id;
	uint16_t children[4] = { 0, 0, 0, 0 };
};

class TerrainQuadTree
{
public:
	TerrainQuadTree(Allocator* allocator);

	void CreateResources(kokko::render::Device* renderDevice, uint8_t levels,
		const TerrainParameters& params);
	void DestroyResources(kokko::render::Device* renderDevice);

	void UpdateTilesToRender(const FrustumPlanes& frustum, const Vec3f& cameraPos,
		const RenderDebugSettings& renderDebug, Array<QuadTreeNodeId>& resultOut);

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
	struct UpdateTilesToRenderParams
	{
		const FrustumPlanes& frustum;
		const Vec3f& cameraPos;
		const RenderDebugSettings& renderDebug;
		Array<QuadTreeNodeId>& resultOut;
	};

	// Returns inserted node index (points to nodes array), or -1 if no insertion
	int BuildQuadTree(const QuadTreeNodeId& id, UpdateTilesToRenderParams& params);
	void RestrictQuadTree();
	void QuadTreeToTiles(uint16_t nodeIndex, UpdateTilesToRenderParams& params);

	static void CreateTileTestData(TerrainTile& tile, int tileX, int tileY, float tileScale);

	static uint16_t TestData(float x, float y);

	static QuadTreeNodeId GetParentId(const QuadTreeNodeId& id);

	Allocator* allocator;
	Array<TerrainQuadTreeNode> nodes;
	TerrainTile* tiles;
	render::TextureId* tileTextureIds;

	uint8_t treeLevels;
	uint8_t maxNodeLevel;
	int tileCount;
	float terrainWidth;
	float terrainBottom;
	float terrainHeight;
};

}
