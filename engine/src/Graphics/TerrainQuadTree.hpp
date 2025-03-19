#pragma once

#include <cstddef>
#include <cstdint>

#include "Core/ArrayView.hpp"
#include "Core/Array.hpp"
#include "Core/HashMap.hpp"
#include "Core/SortedArray.hpp"

#include "Math/Vec3.hpp"

namespace kokko
{
class Allocator;
class RenderDebugSettings;

struct AABB;
struct CameraParameters;
struct FrustumPlanes;
struct Mat4x4f;
struct TerrainTileHeightData;

namespace render
{
struct TextureId;
class Device;
}

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

uint32_t HashValue32(const QuadTreeNodeId& value, uint32_t seed);

struct TerrainTile
{
	static constexpr int QuadsPerSide = 64;
	static constexpr int VerticesPerSide = QuadsPerSide + 1;
	static constexpr int TexelsPerSide = VerticesPerSide + 2;
	static constexpr int TexelsPerTextureRow = TexelsPerSide + 1; // Texel data rows need a stride of 4

	double timeLastUsed = -1.0;
	QuadTreeNodeId id;
	uint16_t minHeight;
	uint16_t maxHeight;
};

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

struct TerrainTileDrawInfo
{
	QuadTreeNodeId id;
	TerrainEdgeType edgeType;
};

class TerrainQuadTree
{
public:
	TerrainQuadTree();
	TerrainQuadTree(Allocator* allocator, render::Device* renderDevice);
	TerrainQuadTree(TerrainQuadTree&& other) noexcept;
	TerrainQuadTree(const TerrainQuadTree&) = delete;
	~TerrainQuadTree();

	TerrainQuadTree& operator=(const TerrainQuadTree&) = delete;
	TerrainQuadTree& operator=(TerrainQuadTree&& other) noexcept;

	void SetHeightmap(const uint16_t* pixels, uint32_t resolution);

	void UpdateTilesToRender(const FrustumPlanes& frustum, const Vec3f& cameraPos,
		const RenderDebugSettings& renderDebug);
	ArrayView<const TerrainTileDrawInfo> GetTilesToRender() const;

	int GetLevelCount() const { return treeLevels; }

	float GetSize() const { return terrainWidth; }
	void SetSize(float size) { terrainWidth = size; }

	float GetBottom() const { return terrainBottom; }
	void SetBottom(float bottom) { terrainBottom = bottom; }

	float GetHeight() const { return terrainHeight; }
	void SetHeight(float height) { terrainHeight = height; }

	float GetLodSizeFactor() const { return lodSizeFactor; }
	void SetLodSizeFactor(float factor) { lodSizeFactor = factor; }

	render::TextureId GetTileHeightTexture(const QuadTreeNodeId& id);

	static uint32_t GetTilesPerDimension(uint8_t level);
	static float GetTileScale(uint8_t level);

private:
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

	struct UpdateTilesToRenderParams
	{
		const FrustumPlanes& frustum;
		const Vec3f& cameraPos;
		const RenderDebugSettings& renderDebug;
	};

	struct EdgeTypeDependents
	{
		uint16_t dependentNodeIndices[2];
		uint32_t numDependents = 0;

		void AddDependent(uint16_t dependent);
	};

	struct HeightCacheEntry
	{
		uint16_t min;
		uint16_t max;
		double lastAccessTime;
	};

	// Returns inserted node index (points to nodes array), or -1 if no insertion
	int BuildQuadTree(const QuadTreeNodeId& id, const UpdateTilesToRenderParams& params);
	void RestrictQuadTree();
	void CalculateEdgeTypes();
	void AddEdgeDependency(const QuadTreeNodeId& dependee, uint16_t dependentNodeIndex);
	void QuadTreeToTiles(uint16_t nodeIndex);
	void LoadTiles();
	void LoadTileData(const QuadTreeNodeId& id, TerrainTileHeightData& heightDataOut);

	Allocator* allocator;
	render::Device* renderDevice;

	Array<TerrainQuadTreeNode> nodes;
	Array<TerrainTileDrawInfo> drawTiles;
	SortedArray<QuadTreeNodeId> parentsToCheck;
	SortedArray<QuadTreeNodeId> neighborsToCheck;
	HashMap<QuadTreeNodeId, EdgeTypeDependents> edgeDependencies; // Key = dependee node, Value = dependent nodes
	HashMap<QuadTreeNodeId, uint32_t> tileIdToIndexMap; // Index into tileData
	HashMap<QuadTreeNodeId, HeightCacheEntry> nodeHeightCache;

	struct TileData
	{
		TerrainTile* tiles = nullptr;
		render::TextureId* textureIds = nullptr;

		void* buffer = nullptr;
		uint32_t count = 0;
		uint32_t textureInitCount = 0; // Textures with storage allocated, but not necessarily in use
		uint32_t allocated = 0;

		TileData() = default;
		TileData(const TileData&) = delete;
		TileData(TileData&& other) noexcept;

		TileData& operator=(const TileData&) = delete;
		TileData& operator=(TileData&& other) noexcept;
	} tileData;

	const uint16_t* heightmapPixels = nullptr;
	uint32_t heightmapSize = 0;

	uint8_t treeLevels = 0;
	uint8_t maxNodeLevel = 0;
	float terrainWidth = 0.0f;
	float terrainBottom = 0.0f;
	float terrainHeight = 0.0f;
	float lodSizeFactor = 0.5f;
	double currentTime = -1.0;
};

}
