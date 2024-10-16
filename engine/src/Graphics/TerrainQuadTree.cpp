#include "Graphics/TerrainQuadTree.hpp"

#include <cassert>

#include "doctest/doctest.h"

#include "Core/SortedArray.hpp"

#include "Debug/Debug.hpp"
#include "Debug/DebugTextRenderer.hpp"
#include "Debug/DebugVectorRenderer.hpp"

#include "Graphics/TerrainSystem.hpp"

#include "Math/AABB.hpp"
#include "Math/Frustum.hpp"
#include "Math/Intersect3D.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/CameraParameters.hpp"
#include "Rendering/RenderDevice.hpp"

#include "System/Time.hpp"

namespace kokko
{

struct TerrainTileHeightData
{
	uint16_t min;
	uint16_t max;
	uint16_t data[TerrainTile::TexelsPerTextureRow * TerrainTile::TexelsPerSide];
};

namespace
{

enum TerrainEdgeMask
{
	TerrainEdgeMask_Regular = 0,
	TerrainEdgeMask_TopSparse = 1 << 0,
	TerrainEdgeMask_RightSparse = 1 << 1,
	TerrainEdgeMask_BottomSparse = 1 << 2,
	TerrainEdgeMask_LeftSparse = 1 << 3,
	TerrainEdgeMask_TopRightSparse = TerrainEdgeMask_TopSparse | TerrainEdgeMask_RightSparse,
	TerrainEdgeMask_RightBottomSparse = TerrainEdgeMask_RightSparse | TerrainEdgeMask_BottomSparse,
	TerrainEdgeMask_BottomLeftSparse = TerrainEdgeMask_BottomSparse | TerrainEdgeMask_LeftSparse,
	TerrainEdgeMask_LeftTopSparse = TerrainEdgeMask_LeftSparse | TerrainEdgeMask_TopSparse
};

TerrainEdgeMask EdgeTypeToMask(TerrainEdgeType type)
{
	switch (type)
	{
	case kokko::TerrainEdgeType::Regular:
		return TerrainEdgeMask_Regular;
	case kokko::TerrainEdgeType::TopSparse:
		return TerrainEdgeMask_TopSparse;
	case kokko::TerrainEdgeType::TopRightSparse:
		return TerrainEdgeMask_TopRightSparse;
	case kokko::TerrainEdgeType::RightSparse:
		return TerrainEdgeMask_RightSparse;
	case kokko::TerrainEdgeType::RightBottomSparse:
		return TerrainEdgeMask_RightBottomSparse;
	case kokko::TerrainEdgeType::BottomSparse:
		return TerrainEdgeMask_BottomSparse;
	case kokko::TerrainEdgeType::BottomLeftSparse:
		return TerrainEdgeMask_BottomLeftSparse;
	case kokko::TerrainEdgeType::LeftSparse:
		return TerrainEdgeMask_LeftSparse;
	case kokko::TerrainEdgeType::LeftTopSparse:
		return TerrainEdgeMask_LeftTopSparse;
	default:
		return TerrainEdgeMask_Regular;
	}
}

TerrainEdgeType EdgeMaskToType(TerrainEdgeMask mask)
{
	switch (mask)
	{
	case TerrainEdgeMask_Regular:
		return TerrainEdgeType::Regular;
	case TerrainEdgeMask_TopSparse:
		return TerrainEdgeType::TopSparse;
	case TerrainEdgeMask_TopRightSparse:
		return TerrainEdgeType::TopRightSparse;
	case TerrainEdgeMask_RightSparse:
		return TerrainEdgeType::RightSparse;
	case TerrainEdgeMask_RightBottomSparse:
		return TerrainEdgeType::RightBottomSparse;
	case TerrainEdgeMask_BottomSparse:
		return TerrainEdgeType::BottomSparse;
	case TerrainEdgeMask_BottomLeftSparse:
		return TerrainEdgeType::BottomLeftSparse;
	case TerrainEdgeMask_LeftSparse:
		return TerrainEdgeType::LeftSparse;
	case TerrainEdgeMask_LeftTopSparse:
		return TerrainEdgeType::LeftTopSparse;
	default:
		return TerrainEdgeType::Regular;
	}
}

QuadTreeNodeId GetParentId(const QuadTreeNodeId& id)
{
	if (id.level == 0)
		return id;

	return QuadTreeNodeId{ id.x / 2, id.y / 2, static_cast<uint8_t>(id.level - 1) };
}

uint8_t GetLevelsFromHeightmapResolution(uint32_t resolution)
{
	uint32_t tilesAtHighestLevel = resolution / TerrainTile::QuadsPerSide;

	uint8_t levels = 0;
	while (tilesAtHighestLevel > 0)
	{
		levels += 1;
		tilesAtHighestLevel /= 2;
	}

	return levels;
}

TEST_CASE("Terrain.GetLevelsFromHeightmapResolution")
{
	CHECK(GetLevelsFromHeightmapResolution(0) == 0);
	CHECK(GetLevelsFromHeightmapResolution(1) == 0);
	CHECK(GetLevelsFromHeightmapResolution(63) == 0);
	CHECK(GetLevelsFromHeightmapResolution(64) == 1);
	CHECK(GetLevelsFromHeightmapResolution(65) == 1);
	CHECK(GetLevelsFromHeightmapResolution(127) == 1);
	CHECK(GetLevelsFromHeightmapResolution(128) == 2);
	CHECK(GetLevelsFromHeightmapResolution(256) == 3);
	CHECK(GetLevelsFromHeightmapResolution(512) == 4);
	CHECK(GetLevelsFromHeightmapResolution(1024) == 5);
	CHECK(GetLevelsFromHeightmapResolution(2048) == 6);
	CHECK(GetLevelsFromHeightmapResolution(4096) == 7);
	CHECK(GetLevelsFromHeightmapResolution(8192) == 8);
}

uint16_t TestData(float x, float y)
{
	float f = 0.02f;
	float a = 0.12f;

	float sum = 0.5f;
	for (int i = 1; i <= 13; i += 6)
	{
		sum += std::sin(x * f * i) * a / i + std::sin(y * f * i) * a / i;
	}
	return static_cast<uint16_t>(sum * UINT16_MAX);
}

void CreateTileTestData(TerrainTileHeightData& tile, uint32_t tileX, uint32_t tileY, float tileScale)
{
	const float quadScale = 1.0f / TerrainTile::QuadsPerSide;

	for (int texY = 0; texY < TerrainTile::TexelsPerSide; ++texY)
	{
		for (int texX = 0; texX < TerrainTile::TexelsPerSide; ++texX)
		{
			float cx = ((texX - 1) * quadScale + tileX) * tileScale;
			float cy = ((texY - 1) * quadScale + tileY) * tileScale;

			int pixelIndex = texY * TerrainTile::TexelsPerTextureRow + texX;
			uint16_t value = TestData(cx, cy);
			tile.min = std::min(tile.min, value);
			tile.max = std::max(tile.max, value);
			tile.data[pixelIndex] = value;
		}
	}
}

} // namespace

uint32_t HashValue32(const QuadTreeNodeId& value, uint32_t seed)
{
	uint32_t hash = HashValue32(&value.x, sizeof(value.x), seed);
	hash = HashValue32(&value.y, sizeof(value.y), hash);
	return HashValue32(&value.level, sizeof(value.level), hash);
}

void TerrainQuadTree::EdgeTypeDependents::AddDependent(uint16_t dependent)
{
	uint32_t idx = numDependents;
	assert(idx < KOKKO_ARRAY_ITEMS(dependentNodeIndices));
	dependentNodeIndices[idx] = dependent;
	numDependents += 1;
}

TerrainQuadTree::TileData::TileData(TileData&& other) noexcept :
	tiles(other.tiles),
	textureIds(other.textureIds),
	buffer(other.buffer),
	count(other.count),
	textureInitCount(other.textureInitCount),
	allocated(other.allocated)
{
	other.tiles = nullptr;
	other.textureIds = nullptr;
	other.buffer = nullptr;
	other.count = 0;
	other.textureInitCount = 0;
	other.allocated = 0;
}

TerrainQuadTree::TileData& TerrainQuadTree::TileData::operator=(TileData&& other) noexcept
{
	tiles = other.tiles;
	textureIds = other.textureIds;
	buffer = other.buffer;
	count = other.count;
	textureInitCount = other.textureInitCount;
	allocated = other.allocated;

	other.tiles = nullptr;
	other.textureIds = nullptr;
	other.buffer = nullptr;
	other.count = 0;
	other.textureInitCount = 0;
	other.allocated = 0;

	return *this;
}

TerrainQuadTree::TerrainQuadTree() :
	allocator(nullptr),
	renderDevice(nullptr),
	nodes(nullptr),
	drawTiles(nullptr),
	parentsToCheck(nullptr),
	neighborsToCheck(nullptr),
	edgeDependencies(nullptr),
	tileIdToIndexMap(nullptr),
	nodeHeightCache(nullptr)
{
}

TerrainQuadTree::TerrainQuadTree(Allocator* allocator, render::Device* renderDevice) :
	allocator(allocator),
	renderDevice(renderDevice),
	nodes(allocator),
	drawTiles(allocator),
	parentsToCheck(allocator),
	neighborsToCheck(allocator),
	edgeDependencies(allocator),
	tileIdToIndexMap(allocator),
	nodeHeightCache(allocator)
{
}

TerrainQuadTree::TerrainQuadTree(TerrainQuadTree&& other) noexcept :
	allocator(other.allocator),
	renderDevice(other.renderDevice),
	nodes(std::move(other.nodes)),
	drawTiles(std::move(other.drawTiles)),
	parentsToCheck(std::move(other.parentsToCheck)),
	neighborsToCheck(std::move(other.neighborsToCheck)),
	edgeDependencies(std::move(other.edgeDependencies)),
	tileIdToIndexMap(std::move(other.tileIdToIndexMap)),
	nodeHeightCache(std::move(other.nodeHeightCache)),
	tileData(std::move(other.tileData)),
	treeLevels(other.treeLevels),
	maxNodeLevel(other.maxNodeLevel),
	terrainWidth(other.terrainWidth),
	terrainBottom(other.terrainBottom),
	terrainHeight(other.terrainHeight),
	lodSizeFactor(other.lodSizeFactor)
{
}

TerrainQuadTree& TerrainQuadTree::operator=(TerrainQuadTree&& other) noexcept
{
	allocator = other.allocator;
	renderDevice = other.renderDevice;
	nodes = std::move(other.nodes);
	drawTiles = std::move(other.drawTiles);
	parentsToCheck = std::move(other.parentsToCheck);
	neighborsToCheck = std::move(other.neighborsToCheck);
	edgeDependencies = std::move(other.edgeDependencies);
	tileIdToIndexMap = std::move(other.tileIdToIndexMap);
	nodeHeightCache = std::move(other.nodeHeightCache);
	tileData = std::move(other.tileData);
	treeLevels = other.treeLevels;
	maxNodeLevel = other.maxNodeLevel;
	terrainWidth = other.terrainWidth;
	terrainBottom = other.terrainBottom;
	terrainHeight = other.terrainHeight;
	lodSizeFactor = other.lodSizeFactor;

	return *this;
}

TerrainQuadTree::~TerrainQuadTree()
{
	KOKKO_PROFILE_FUNCTION();

	if (renderDevice != nullptr)
	{
		auto scope = renderDevice->CreateDebugScope(0, ConstStringView("TerrainQuadTree_Destruct"));

		if (tileData.textureIds != nullptr)
			renderDevice->DestroyTextures(tileData.count, tileData.textureIds);
	}

	if (tileData.buffer != nullptr)
		allocator->Deallocate(tileData.buffer);
}

void TerrainQuadTree::SetHeightmap(const uint16_t* pixels, uint32_t resolution)
{
	if (pixels != heightmapPixels || resolution != heightmapSize)
	{
		tileData.count = 0;
		tileIdToIndexMap.Clear();
		nodeHeightCache.Clear();

		treeLevels = GetLevelsFromHeightmapResolution(resolution);
		heightmapPixels = pixels;
		heightmapSize = resolution;
	}
}

void TerrainQuadTree::UpdateTilesToRender(
	const FrustumPlanes& frustum,
	const Vec3f& cameraPos,
	const RenderDebugSettings& renderDebug)
{
	KOKKO_PROFILE_SCOPE("TerrainQuadTree::UpdateTilesToRender()");

	nodes.Clear();
	drawTiles.Clear();
	maxNodeLevel = 0;

	// Used for cache eviction
	currentTime = Time::GetRunningTime();

	// Calculates optimal set of tiles to render and updates quad tree <nodes>
	UpdateTilesToRenderParams params{ frustum, cameraPos, renderDebug };
	int rootNodeIndex = BuildQuadTree(QuadTreeNodeId{}, params);
	if (rootNodeIndex == -1)
		return;

	// Next we need to update the quad tree so that it forms a restricted quad tree
	RestrictQuadTree();

	// Calculate edge types for rendering
	CalculateEdgeTypes();

	// Then we create the final render tiles from the leaf nodes of the quad tree
	QuadTreeToTiles(rootNodeIndex);

	// Load visible tiles
	LoadTiles();
}

ArrayView<const TerrainTileDrawInfo> TerrainQuadTree::GetTilesToRender() const
{
	return ArrayView(drawTiles.GetData(), drawTiles.GetCount());
}

int TerrainQuadTree::BuildQuadTree(const QuadTreeNodeId& id, const UpdateTilesToRenderParams& params)
{
	auto cacheKv = nodeHeightCache.Lookup(id);
	QuadTreeNodeId searchId = id;
	while (cacheKv == nullptr && searchId.level != 0)
	{
		searchId = GetParentId(searchId);
		cacheKv = nodeHeightCache.Lookup(searchId);
	}

	float minHeight = terrainBottom;
	float maxHeight = terrainBottom + terrainHeight;
	if (cacheKv != nullptr)
	{
		HeightCacheEntry& entry = cacheKv->second;
		minHeight = entry.min / static_cast<float>(UINT16_MAX) * terrainHeight + terrainBottom;
		maxHeight = entry.max / static_cast<float>(UINT16_MAX) * terrainHeight + terrainBottom;
		entry.lastAccessTime = currentTime;
	}

	float tileScale = GetTileScale(id.level);
	float tileWidth = terrainWidth * tileScale;
	float halfSize = terrainWidth * 0.5f;
	Vec3f tileMin(id.x * tileWidth - halfSize, minHeight, id.y * tileWidth - halfSize);
	Vec3f tileSize(tileWidth, maxHeight - minHeight, tileWidth);

	AABB tileBounds;
	tileBounds.extents = tileSize * 0.5f;
	tileBounds.center = tileMin + tileBounds.extents;

	if (Intersect::FrustumAabb(params.frustum, tileBounds) == false)
		return -1;

	bool lastLevel = id.level + 1 == treeLevels;
	bool tileIsSmallEnough = lastLevel || (tileWidth < (tileBounds.center - params.cameraPos).Magnitude() * lodSizeFactor);

	int nodeIndex = static_cast<int>(nodes.GetCount());
	assert(nodeIndex <= UINT16_MAX);
	{
		TerrainQuadTreeNode& node = nodes.PushBack();
		node.id = id;

		maxNodeLevel = std::max(maxNodeLevel, id.level);
	}

	if (tileIsSmallEnough)
	{
		if (params.renderDebug.IsFeatureEnabled(RenderDebugFeatureFlag::DrawTerrainTiles))
		{
			Vec3f scale = tileBounds.extents * 2.0f;
			scale.y = 0;

			Mat4x4f transform = Mat4x4f::Translate(tileBounds.center) * Mat4x4f::Scale(scale);

			float alpha = 1.0f - (treeLevels - id.level) / static_cast<float>(treeLevels);
			Color color(1.0f, 0.0f, 1.0f, alpha * alpha);
			Debug::Get()->GetVectorRenderer()->DrawWireCube(transform, color);
		}
	}
	else
	{
		for (int y = 0; y < 2; ++y)
		{
			for (int x = 0; x < 2; ++x)
			{
				QuadTreeNodeId tileId{ id.x * 2 + x, id.y * 2 + y, static_cast<uint8_t>(id.level + 1) };

				int childIndex = BuildQuadTree(tileId, params);
				if (childIndex >= 0)
					nodes[nodeIndex].children[y * 2 + x] = static_cast<uint16_t>(childIndex);
			}
		}
	}

	return nodeIndex;
}

void TerrainQuadTree::RestrictQuadTree()
{
	KOKKO_PROFILE_SCOPE("TerrainQuadTree::RestrictQuadTree()");

	for (uint8_t currentLevel = maxNodeLevel; currentLevel > 1; --currentLevel)
	{
		for (const auto& node : nodes)
			if (node.id.level == currentLevel)
				parentsToCheck.InsertUnique(GetParentId(node.id));

		uint32_t tilesPerDim = GetTilesPerDimension(currentLevel - 1);
		for (const auto& id : parentsToCheck)
		{
			// Bitwise AND is used to check if the potential tile has a different parent from current tile

			if ((id.x & 1) == 0 && id.x > 0)
				neighborsToCheck.InsertUnique(QuadTreeNodeId{ id.x - 1, id.y, id.level });
			if ((id.x & 1) == 1 && id.x + 1 < tilesPerDim)
				neighborsToCheck.InsertUnique(QuadTreeNodeId{ id.x + 1, id.y, id.level });
			if ((id.y & 1) == 0 && id.y > 0)
				neighborsToCheck.InsertUnique(QuadTreeNodeId{ id.x, id.y - 1, id.level });
			if ((id.y & 1) == 1 && id.y + 1 < tilesPerDim)
				neighborsToCheck.InsertUnique(QuadTreeNodeId{ id.x, id.y + 1, id.level });
		}

		for (const auto& id : neighborsToCheck)
		{
			// Try to find node in nodes
			// If not, split parent tiles until we have the node

			int currentNodeIdx = 0;
			for (uint8_t level = 0; level <= id.level; ++level)
			{
				if (level == id.level)
					break;

				uint8_t childLevel = static_cast<uint8_t>(level + 1);
				int levelDiff = id.level - childLevel;
				int childX = id.x >> levelDiff;
				int childY = id.y >> levelDiff;
				int childIdx = (childY & 1) * 2 + (childX & 1);

				// Verify next level towards <id> exists
				// If current node has no children, split
				if (nodes[currentNodeIdx].HasChildren() == false)
				{
					for (int y = 0; y < 2; ++y)
					{
						for (int x = 0; x < 2; ++x)
						{
							// Create child node
							size_t newNodeIdx = nodes.GetCount();
							assert(newNodeIdx <= UINT16_MAX);
							TerrainQuadTreeNode& newNode = nodes.PushBack();
							const QuadTreeNodeId& curId = nodes[currentNodeIdx].id;
							newNode.id = QuadTreeNodeId{ curId.x * 2 + x, curId.y * 2 + y, childLevel };
							nodes[currentNodeIdx].children[y * 2 + x] = static_cast<uint16_t>(newNodeIdx);
						}
					}
				}
				// If it has some children, but not our target tile, skip work on it
				else if (nodes[currentNodeIdx].children[childIdx] == 0)
					break;

				// Update currentNodeIdx and continue
				currentNodeIdx = nodes[currentNodeIdx].children[childIdx];
				continue;
			}
		}

		parentsToCheck.Clear();
		neighborsToCheck.Clear();
	}
}

void TerrainQuadTree::CalculateEdgeTypes()
{
	KOKKO_PROFILE_SCOPE("TerrainQuadTree::CalculateEdgeTypes()");

	for (uint16_t nodeIndex = 0, nodeCount = nodes.GetCount(); nodeIndex != nodeCount; ++nodeIndex)
	{
		const TerrainQuadTreeNode& node = nodes[nodeIndex];
		if (node.HasChildren() == false)
		{
			QuadTreeNodeId id = node.id;
			uint32_t tilesPerDim = GetTilesPerDimension(id.level);
			if ((id.x & 1) == 0 && id.x > 0)
				AddEdgeDependency(QuadTreeNodeId{ id.x - 1, id.y, id.level }, nodeIndex);
			if ((id.x & 1) == 1 && id.x + 1 < tilesPerDim)
				AddEdgeDependency(QuadTreeNodeId{ id.x + 1, id.y, id.level }, nodeIndex);
			if ((id.y & 1) == 0 && id.y > 0)
				AddEdgeDependency(QuadTreeNodeId{ id.x, id.y - 1, id.level }, nodeIndex);
			if ((id.y & 1) == 1 && id.y + 1 < tilesPerDim)
				AddEdgeDependency(QuadTreeNodeId{ id.x, id.y + 1, id.level }, nodeIndex);
		}
	}

	for (auto& pair : edgeDependencies)
	{
		const QuadTreeNodeId& dependee = pair.first;
		// Try to find node in nodes
		// If not, mark edge status as sparse

		TerrainQuadTreeNode* currentNode = &nodes[0];
		for (uint8_t level = 0; level <= dependee.level; ++level)
		{
			if (level == dependee.level)
				break;

			uint8_t childLevel = static_cast<uint8_t>(level + 1);
			int levelDiff = dependee.level - childLevel;
			int childX = dependee.x >> levelDiff;
			int childY = dependee.y >> levelDiff;
			int childIndex = (childY & 1) * 2 + (childX & 1);

			// Verify next level towards <id> exists
			if (currentNode->HasChildren() == false)
			{
				// Mark dependent edge as sparse
				EdgeTypeDependents& dependents = pair.second;
				for (uint32_t i = 0; i < dependents.numDependents; ++i)
				{
					TerrainQuadTreeNode& dependentNode = nodes[dependents.dependentNodeIndices[i]];
					int edgeMask = EdgeTypeToMask(dependentNode.edgeType);
					int diffX = dependee.x - dependentNode.id.x;
					int diffY = dependee.y - dependentNode.id.y;

					if (diffX != 0)
						edgeMask |= diffX < 0 ? TerrainEdgeMask_LeftSparse : TerrainEdgeMask_RightSparse;
					else
						edgeMask |= diffY < 0 ? TerrainEdgeMask_TopSparse : TerrainEdgeMask_BottomSparse;

					dependentNode.edgeType = EdgeMaskToType(static_cast<TerrainEdgeMask>(edgeMask));
				}
			}
			// If it has some children, but not our target tile, skip work on it
			else if (currentNode->children[childIndex] == 0)
				break;

			// Update currentNode and continue
			int childNodeIndex = currentNode->children[childIndex];

			currentNode = &nodes[childNodeIndex];
		}
	}

	edgeDependencies.Clear();
}

void TerrainQuadTree::AddEdgeDependency(const QuadTreeNodeId& dependee, uint16_t dependentNodeIndex)
{
	auto pair = edgeDependencies.Lookup(dependee);
	if (pair == nullptr)
		pair = edgeDependencies.Insert(dependee);

	pair->second.AddDependent(dependentNodeIndex);
}

void TerrainQuadTree::QuadTreeToTiles(uint16_t nodeIndex)
{
	const TerrainQuadTreeNode& node = nodes[nodeIndex];

	if (node.HasChildren() == false)
	{
		drawTiles.PushBack(TerrainTileDrawInfo{ node.id, node.edgeType });
		return;
	}

	for (int y = 0; y < 2; ++y)
	{
		for (int x = 0; x < 2; ++x)
		{
			uint16_t childIndex = node.children[y * 2 + x];
			if (childIndex != 0)
				QuadTreeToTiles(childIndex);
		}
	}
}

void TerrainQuadTree::LoadTiles()
{
	const double oldTileThresholdTime = currentTime - 10.0;

	// Unload tiles that haven't been used for a while
	for (uint32_t i = 0; i < tileData.count; )
	{
		if (tileData.tiles[i].timeLastUsed < oldTileThresholdTime)
		{
			QuadTreeNodeId deletedTileId = tileData.tiles[i].id;
			if (i + 1 < tileData.count) // Move the last tile into this slot
			{
				uint32_t moveIdx = tileData.count - 1;
				QuadTreeNodeId movedTileId = tileData.tiles[moveIdx].id;

				tileData.tiles[i] = tileData.tiles[moveIdx];

				// Swap texture IDs
				render::TextureId tempTexture = tileData.textureIds[i];
				tileData.textureIds[i] = tileData.textureIds[moveIdx];
				tileData.textureIds[moveIdx] = tempTexture;

				// Update moved tile's ID to index mapping
				if (auto pair = tileIdToIndexMap.Lookup(movedTileId))
					pair->second = i;
			}

			// Remove deleted tile's ID to index mapping
			if (auto pair = tileIdToIndexMap.Lookup(deletedTileId))
				tileIdToIndexMap.Remove(pair);

			tileData.count -= 1;
			continue;
		}
		
		i += 1;
	}

	// Load new tiles that are missing

	size_t tileCount = drawTiles.GetCount();

	uint32_t missingTiles = 0;
	for (const TerrainTileDrawInfo& tile : drawTiles)
	{
		auto pair = tileIdToIndexMap.Lookup(tile.id);
		if (pair == nullptr)
			missingTiles += 1;
	}
	const uint32_t required = tileData.count + missingTiles;

	if (required > tileData.allocated)
	{
		// Reallocate
		uint32_t newAllocated = Math::RoundUpToMultiple(required * 4 / 3, 16u);
		size_t bytes = newAllocated * (sizeof(TerrainTile) + sizeof(render::TextureId));

		TileData newData;
		newData.buffer = allocator->Allocate(bytes, "TerrainQuadTree.tileData.buffer");
		newData.count = tileData.count;
		newData.textureInitCount = tileData.textureInitCount;
		newData.allocated = newAllocated;

		newData.tiles = static_cast<TerrainTile*>(newData.buffer);
		newData.textureIds = reinterpret_cast<render::TextureId*>(newData.tiles + newAllocated);

		if (tileData.buffer != nullptr)
		{
			std::memcpy(newData.tiles, tileData.tiles, tileData.count * sizeof(TerrainTile));
			std::memcpy(newData.textureIds, tileData.textureIds, tileData.textureInitCount * sizeof(render::TextureId));

			allocator->Deallocate(tileData.buffer);
		}

		tileData = std::move(newData);
	}

	constexpr int texResolution = TerrainTile::TexelsPerSide;
	if (required > tileData.textureInitCount)
	{
		render::TextureId* newTextures = &tileData.textureIds[tileData.textureInitCount];
		uint32_t newTextureCount = required - tileData.textureInitCount;
		renderDevice->CreateTextures(RenderTextureTarget::Texture2d, newTextureCount, newTextures);

		for (uint32_t i = tileData.textureInitCount; i < required; ++i)
			renderDevice->SetTextureStorage2D(
				tileData.textureIds[i], 1, RenderTextureSizedFormat::R16, texResolution, texResolution);

		tileData.textureInitCount = required;
	}

	TerrainTileHeightData heightData;

	for (const TerrainTileDrawInfo& drawTile : drawTiles)
	{
		auto pair = tileIdToIndexMap.Lookup(drawTile.id);
		if (pair != nullptr)
		{
			tileData.tiles[pair->second].timeLastUsed = currentTime;
		}
		else
		{
			uint32_t tileIdx = tileData.count;

			LoadTileData(drawTile.id, heightData);

			TerrainTile& tile = tileData.tiles[tileIdx];
			tile.id = drawTile.id;
			tile.timeLastUsed = currentTime;
			tile.minHeight = heightData.min;
			tile.maxHeight = heightData.max;

			auto cacheKv = nodeHeightCache.Insert(drawTile.id);
			cacheKv->second.min = heightData.min;
			cacheKv->second.max = heightData.max;
			cacheKv->second.lastAccessTime = currentTime;

			renderDevice->SetTextureSubImage2D(tileData.textureIds[tileIdx], 0, 0, 0,
				texResolution, texResolution, RenderTextureBaseFormat::R,
				RenderTextureDataType::UnsignedShort, heightData.data);

			pair = tileIdToIndexMap.Insert(drawTile.id);
			pair->second = tileIdx;
			tileData.count += 1;
		}
	}

	// Unload old height cache entries

	const double oldCacheThresholdTime = currentTime - 300.0;

	for (auto itr = nodeHeightCache.begin(), end = nodeHeightCache.end(); itr != end; ++itr)
	{
		if (itr->second.lastAccessTime < oldCacheThresholdTime)
		{
			nodeHeightCache.Remove(itr);
			break; // Only remove one entry per update as removal invalidates iterators
		}
	}
}

void TerrainQuadTree::LoadTileData(const QuadTreeNodeId& id, TerrainTileHeightData& heightDataOut)
{
	if (heightmapPixels == nullptr)
	{
		CreateTileTestData(heightDataOut, id.x, id.y, GetTileScale(id.level));
		return;
	}

	const uint32_t tilesPerDimension = GetTilesPerDimension(id.level);
	const uint32_t pixelsPerTile = heightmapSize / tilesPerDimension;
	const uint32_t pixelsPerQuad = pixelsPerTile / TerrainTile::QuadsPerSide;

	uint16_t min = UINT16_MAX;
	uint16_t max = 0;

	for (int outputY = 0; outputY < TerrainTile::TexelsPerSide; ++outputY)
	{
		for (int outputX = 0; outputX < TerrainTile::TexelsPerSide; ++outputX)
		{
			int inputX = pixelsPerTile * id.x + pixelsPerQuad * (outputX - 1);
			int inputY = pixelsPerTile * id.y + pixelsPerQuad * (outputY - 1);
			int clampedX = std::clamp(inputX, 0, static_cast<int>(heightmapSize - 1));
			int clampedY = std::clamp(inputY, 0, static_cast<int>(heightmapSize - 1));
			int inputIdx = clampedY * heightmapSize + clampedX;

			int outputIdx = outputY * TerrainTile::TexelsPerTextureRow + outputX;
			uint16_t val = heightmapPixels[inputIdx];

			min = std::min(min, val);
			max = std::max(max, val);
			heightDataOut.data[outputIdx] = val;
		}
	}

	heightDataOut.min = min;
	heightDataOut.max = max;
}

render::TextureId TerrainQuadTree::GetTileHeightTexture(const QuadTreeNodeId& id)
{
	auto pair = tileIdToIndexMap.Lookup(id);
	if (pair != nullptr)
		return tileData.textureIds[pair->second];
	assert(pair != nullptr);
	return render::TextureId::Null;
}

uint32_t TerrainQuadTree::GetTilesPerDimension(uint8_t level)
{
	assert(level >= 0);
	assert(level < 31);
	return 1 << level;
}

TEST_CASE("TerrainQuadTree.GetTilesPerDimension")
{
	CHECK(TerrainQuadTree::GetTilesPerDimension(0) == 1);
	CHECK(TerrainQuadTree::GetTilesPerDimension(1) == 2);
	CHECK(TerrainQuadTree::GetTilesPerDimension(2) == 4);
	CHECK(TerrainQuadTree::GetTilesPerDimension(3) == 8);
}

float TerrainQuadTree::GetTileScale(uint8_t level)
{
	return 1.0f / (1 << level);
}

TEST_CASE("TerrainQuadTree.GetTileScale")
{
	CHECK(TerrainQuadTree::GetTileScale(0) == doctest::Approx(1.0f));
	CHECK(TerrainQuadTree::GetTileScale(1) == doctest::Approx(0.5f));
	CHECK(TerrainQuadTree::GetTileScale(2) == doctest::Approx(0.25f));
	CHECK(TerrainQuadTree::GetTileScale(3) == doctest::Approx(0.125f));
}

} // namespace kokko
