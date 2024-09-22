#pragma once

#include <cstddef>
#include <cstdint>

#include "Core/Array.hpp"
#include "Core/HashMap.hpp"
#include "Core/SortedArray.hpp"

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
	uint32_t x = 0;
	uint32_t y = 0;
	uint8_t level = 0;

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

uint32_t Hash32(const QuadTreeNodeId& value, uint32_t seed);

enum class TerrainEdgeType : uint8_t
{
	Regular,
	TopSparse,
	TopRightSparse,
	RightSparse,
	RightBottomSparse,
	BottomSparse,
	BottomLeftSparse,
	LeftSparse,
	LeftTopSparse
};

struct TerrainQuadTreeNode
{
	QuadTreeNodeId id;
	uint16_t children[4] = { 0, 0, 0, 0 };
	TerrainEdgeType edgeType = TerrainEdgeType::Regular;

	uint16_t HasChildren() const
	{
		return children[0] != 0 || children[1] != 0 || children[2] != 0 || children[3] != 0;
	}
};

struct TerrainTileDrawInfo
{
	QuadTreeNodeId id;
	TerrainEdgeType edgeType;
};

class TerrainQuadTree
{
public:
	TerrainQuadTree(Allocator* allocator, render::Device* renderDevice);
	TerrainQuadTree(const TerrainQuadTree&) = delete;
	~TerrainQuadTree();

	void CreateResources(uint8_t levels, const TerrainParameters& params);

	void UpdateTilesToRender(const FrustumPlanes& frustum, const Vec3f& cameraPos,
		const RenderDebugSettings& renderDebug, Array<TerrainTileDrawInfo>& resultOut);

	int GetLevelCount() const;

	float GetSize() const { return terrainWidth; }
	void SetSize(float size) { terrainWidth = size; }

	float GetBottom() const { return terrainBottom; }
	void SetBottom(float bottom) { terrainBottom = bottom; }

	float GetHeight() const { return terrainHeight; }
	void SetHeight(float height) { terrainHeight = height; }

	const TerrainTile* GetTile(uint8_t level, int x, int y);
	render::TextureId GetTileHeightTexture(uint8_t level, int x, int y);

	static uint32_t GetTilesPerDimension(uint8_t level);
	static int GetTileIndex(uint8_t level, int x, int y);
	static int GetTileCountForLevelCount(uint8_t levelCount);
	static float GetTileScale(uint8_t level);

private:
	struct UpdateTilesToRenderParams
	{
		const FrustumPlanes& frustum;
		const Vec3f& cameraPos;
		const RenderDebugSettings& renderDebug;
		Array<TerrainTileDrawInfo>& resultOut;
	};

	struct EdgeTypeDependents
	{
		uint16_t dependentNodeIndices[2];
		uint32_t numDependents = 0;

		void AddDependent(uint16_t dependent);
	};

	// Returns inserted node index (points to nodes array), or -1 if no insertion
	int BuildQuadTree(const QuadTreeNodeId& id, const UpdateTilesToRenderParams& params);
	void RestrictQuadTree();
	void CalculateEdgeTypes();
	void AddEdgeDependency(const QuadTreeNodeId& dependee, uint16_t dependentNodeIndex);
	void QuadTreeToTiles(uint16_t nodeIndex, UpdateTilesToRenderParams& params);

	static void CreateTileTestData(TerrainTile& tile, int tileX, int tileY, float tileScale);

	static uint16_t TestData(float x, float y);

	static QuadTreeNodeId GetParentId(const QuadTreeNodeId& id);

	Allocator* allocator;
	render::Device* renderDevice;
	Array<TerrainQuadTreeNode> nodes;
	SortedArray<QuadTreeNodeId> parentsToCheck;
	SortedArray<QuadTreeNodeId> neighborsToCheck;
	HashMap<QuadTreeNodeId, EdgeTypeDependents> edgeDependencies; // Key = dependee node, Value = dependent nodes
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
