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

namespace kokko
{

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

TerrainQuadTree::TerrainQuadTree(Allocator* allocator, render::Device* renderDevice) :
	allocator(allocator),
	renderDevice(renderDevice),
	nodes(allocator),
	parentsToCheck(allocator),
	neighborsToCheck(allocator),
	edgeDependencies(allocator),
	tiles(nullptr),
	tileTextureIds(nullptr),
	treeLevels(0),
	maxNodeLevel(0),
	tileCount(0),
	terrainWidth(0.0f),
	terrainBottom(0.0f),
	terrainHeight(0.0f)
{
}

TerrainQuadTree::~TerrainQuadTree()
{
	if (tileTextureIds != nullptr)
		renderDevice->DestroyTextures(static_cast<uint32_t>(tileCount), tileTextureIds);

	allocator->Deallocate(tiles);
	allocator->Deallocate(tileTextureIds);
}

void TerrainQuadTree::CreateResources(uint8_t levels, const TerrainParameters& params)
{
	constexpr int texResolution = TerrainTile::TexelsPerSide;

	treeLevels = levels;
	tileCount = GetTileCountForLevelCount(levels);

	terrainWidth = params.terrainSize;
	terrainBottom = params.heightOrigin;
	terrainHeight = params.heightRange;

	void* buffer = allocator->Allocate(tileCount * sizeof(TerrainTile), "TerrainQuadTree.tiles");
	tiles = static_cast<TerrainTile*>(buffer);

	for (int levelIdx = 0; levelIdx < treeLevels; ++levelIdx)
	{
		int tilesPerDimension = GetTilesPerDimension(levelIdx);

		for (int tileY = 0; tileY < tilesPerDimension; ++tileY)
		{
			for (int tileX = 0; tileX < tilesPerDimension; ++tileX)
			{
				float tileScale = terrainWidth / tilesPerDimension;
				int tileIdx = GetTileIndex(levelIdx, tileX, tileY);
				TerrainTile& tile = tiles[tileIdx];

				CreateTileTestData(tile, tileX, tileY, tileScale);
			}
		}
	}

	tileTextureIds = static_cast<render::TextureId*>(
		allocator->Allocate(tileCount * sizeof(uint32_t), "TerrainQuadTree.tileTextureIds"));
	renderDevice->CreateTextures(RenderTextureTarget::Texture2d, tileCount, tileTextureIds);

	for (uint8_t levelIdx = 0; levelIdx < treeLevels; ++levelIdx)
	{
		int tilesPerDimension = GetTilesPerDimension(levelIdx);

		for (int tileY = 0; tileY < tilesPerDimension; ++tileY)
		{
			for (int tileX = 0; tileX < tilesPerDimension; ++tileX)
			{
				int tileIdx = GetTileIndex(levelIdx, tileX, tileY);
				
				renderDevice->SetTextureStorage2D(
					tileTextureIds[tileIdx], 1, RenderTextureSizedFormat::R16, texResolution, texResolution);

				renderDevice->SetTextureSubImage2D(tileTextureIds[tileIdx], 0, 0, 0,
					texResolution, texResolution, RenderTextureBaseFormat::R,
					RenderTextureDataType::UnsignedShort, tiles[tileIdx].heightData);
			}
		}
	}
}

void TerrainQuadTree::UpdateTilesToRender(
	const FrustumPlanes& frustum,
	const Vec3f& cameraPos,
	const RenderDebugSettings& renderDebug,
	Array<TerrainTileDrawInfo>& resultOut)
{
	KOKKO_PROFILE_SCOPE("TerrainQuadTree::UpdateTilesToRender()");

	nodes.Clear();
	maxNodeLevel = 0;

	// Calculates optimal set of tiles to render and updates quad tree <nodes>
	UpdateTilesToRenderParams params{ frustum, cameraPos, renderDebug, resultOut };
	int rootNodeIndex = BuildQuadTree(QuadTreeNodeId{}, params);

	size_t oldNodeCount = nodes.GetCount();

	// Next we need to update the quad tree so that it forms a restricted quad tree
	RestrictQuadTree();

	CalculateEdgeTypes();

	// Then we create the final render tiles from the leaf nodes of the quad tree
	QuadTreeToTiles(rootNodeIndex, params);
}

int TerrainQuadTree::BuildQuadTree(const QuadTreeNodeId& id, const UpdateTilesToRenderParams& params)
{
	float tileScale = GetTileScale(id.level);

	float halfTerrainSize = terrainWidth * 0.5f;
	Vec3f levelOrigin(-halfTerrainSize, 0.0f, -halfTerrainSize);
	float tileWidth = terrainWidth * tileScale;
	Vec3f tileMin = levelOrigin + Vec3f(id.x * tileWidth, terrainBottom, id.y * tileWidth);
	Vec3f tileSize(tileWidth, terrainHeight, tileWidth);

	AABB tileBounds;
	tileBounds.extents = tileSize * 0.5f;
	tileBounds.center = tileMin + tileBounds.extents;

	if (Intersect::FrustumAabb(params.frustum, tileBounds) == false)
		return -1;
	
	constexpr float sizeFactor = 0.5f;

	bool lastLevel = id.level + 1 == treeLevels;
	bool tileIsSmallEnough = lastLevel || (tileWidth < (tileBounds.center - params.cameraPos).Magnitude() * sizeFactor);

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
			Vec3f scale = tileSize;
			scale.y = 0.0f;

			Mat4x4f transform = Mat4x4f::Translate(tileBounds.center) * Mat4x4f::Scale(scale);

			Debug::Get()->GetVectorRenderer()->DrawWireCube(transform, Color(1.0f, 0.0f, 1.0f));
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

			TerrainQuadTreeNode* currentNode = &nodes[0];
			for (uint8_t level = 0; level <= id.level; ++level)
			{
				if (level == id.level)
					break;

				uint8_t childLevel = static_cast<uint8_t>(level + 1);
				int levelDiff = id.level - childLevel;
				int childX = id.x >> levelDiff;
				int childY = id.y >> levelDiff;
				int childIndex = (childY & 1) * 2 + (childX & 1);

				// Verify next level towards <id> exists
				// If currentNode has no children, split
				bool hasChildren = false;
				for (int i = 0; i < 4; ++i)
					if (currentNode->children[i] != 0)
						hasChildren = true;

				if (hasChildren == false)
				{
					for (int y = 0; y < 2; ++y)
					{
						for (int x = 0; x < 2; ++x)
						{
							// Create child node
							size_t newNodeIndex = nodes.GetCount();
							assert(newNodeIndex <= UINT16_MAX);
							TerrainQuadTreeNode& newNode = nodes.PushBack();
							const QuadTreeNodeId& curId = currentNode->id;
							newNode.id = QuadTreeNodeId{ curId.x * 2 + x, curId.y * 2 + y, childLevel };
							currentNode->children[y * 2 + x] = static_cast<uint16_t>(newNodeIndex);
						}
					}
				}
				// If it has some children, but not our target tile, skip work on it
				else if (currentNode->children[childIndex] == 0)
					break;

				// Update currentNode and continue
				int childNodeIndex = currentNode->children[childIndex];

				currentNode = &nodes[childNodeIndex];
				continue;
			}
		}

		parentsToCheck.Clear();
		neighborsToCheck.Clear();
	}
}

void TerrainQuadTree::CalculateEdgeTypes()
{
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

void TerrainQuadTree::QuadTreeToTiles(uint16_t nodeIndex, UpdateTilesToRenderParams& params)
{
	const TerrainQuadTreeNode& node = nodes[nodeIndex];

	bool hasChildren = false;
	for (int i = 0; i < 4; ++i)
		if (node.children[i] != 0)
			hasChildren = true;

	if (hasChildren == false)
	{
		params.resultOut.PushBack(TerrainTileDrawInfo{ node.id, node.edgeType });
		return;
	}

	for (int y = 0; y < 2; ++y)
	{
		for (int x = 0; x < 2; ++x)
		{
			uint16_t childIndex = node.children[y * 2 + x];
			if (childIndex != 0)
				QuadTreeToTiles(childIndex, params);
		}
	}
}

int TerrainQuadTree::GetLevelCount() const
{
	return treeLevels;
}

const TerrainTile* TerrainQuadTree::GetTile(uint8_t level, int x, int y)
{
	return &tiles[GetTileIndex(level, x, y)];
}

render::TextureId TerrainQuadTree::GetTileHeightTexture(uint8_t level, int x, int y)
{
	return tileTextureIds[GetTileIndex(level, x, y)];
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

int TerrainQuadTree::GetTileIndex(uint8_t level, int x, int y)
{
	int levelStart = 0;
	int levelSize = 1;

	for (int i = 0; i < level; ++i)
	{
		levelStart += levelSize * levelSize;
		levelSize *= 2;
	}

	return levelStart + y * levelSize + x;
}

TEST_CASE("TerrainQuadTree.GetTileIndex")
{
	CHECK(TerrainQuadTree::GetTileIndex(0, 0, 0) == 0);
	CHECK(TerrainQuadTree::GetTileIndex(1, 0, 0) == 1);
	CHECK(TerrainQuadTree::GetTileIndex(1, 1, 0) == 2);
	CHECK(TerrainQuadTree::GetTileIndex(1, 0, 1) == 3);
	CHECK(TerrainQuadTree::GetTileIndex(1, 1, 1) == 4);
	CHECK(TerrainQuadTree::GetTileIndex(2, 0, 0) == 5);
	CHECK(TerrainQuadTree::GetTileIndex(2, 1, 0) == 6);
	CHECK(TerrainQuadTree::GetTileIndex(2, 0, 1) == 9);
	CHECK(TerrainQuadTree::GetTileIndex(3, 0, 0) == 21);
}

int TerrainQuadTree::GetTileCountForLevelCount(uint8_t levelCount)
{
	return GetTileIndex(levelCount, 0, 0);
}

TEST_CASE("TerrainQuadTree.GetTileCountForLevelCount")
{
	CHECK(TerrainQuadTree::GetTileCountForLevelCount(0) == 0);
	CHECK(TerrainQuadTree::GetTileCountForLevelCount(1) == 1);
	CHECK(TerrainQuadTree::GetTileCountForLevelCount(2) == 5);
	CHECK(TerrainQuadTree::GetTileCountForLevelCount(3) == 21);
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

void TerrainQuadTree::CreateTileTestData(TerrainTile& tile, int tileX, int tileY, float tileScale)
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
			tile.heightData[pixelIndex] = value;
		}
	}
}

uint16_t TerrainQuadTree::TestData(float x, float y)
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

QuadTreeNodeId TerrainQuadTree::GetParentId(const QuadTreeNodeId& id)
{
	if (id.level == 0)
		return id;

	return QuadTreeNodeId{ id.x / 2, id.y / 2, static_cast<uint8_t>(id.level - 1) };
}

} // namespace kokko
