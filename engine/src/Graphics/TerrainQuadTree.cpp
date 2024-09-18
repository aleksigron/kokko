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

TerrainQuadTree::TerrainQuadTree(Allocator* allocator) :
	allocator(allocator),
	nodes(allocator),
	tiles(nullptr),
	tileTextureIds(nullptr),
	treeLevels(0),
	tileCount(0),
	terrainWidth(0.0f),
	terrainBottom(0.0f),
	terrainHeight(0.0f)
{
}

void TerrainQuadTree::CreateResources(render::Device* renderDevice, uint8_t levels,
	const TerrainParameters& params)
{
	constexpr int tileResolution = TerrainTile::Resolution;
	constexpr int texResolution = TerrainTile::ResolutionWithBorder;

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

void TerrainQuadTree::DestroyResources(render::Device* renderDevice)
{
	if (tileTextureIds != nullptr)
		renderDevice->DestroyTextures(static_cast<unsigned int>(tileCount), tileTextureIds);

	allocator->Deallocate(tiles);
	allocator->Deallocate(tileTextureIds);
}

void TerrainQuadTree::UpdateTilesToRender(
	const FrustumPlanes& frustum,
	const Vec3f& cameraPos,
	const RenderDebugSettings& renderDebug,
	Array<QuadTreeNodeId>& resultOut)
{
	KOKKO_PROFILE_SCOPE("TerrainQuadTree::UpdateTilesToRender()");

	nodes.Clear();
	maxNodeLevel = 0;

	// Calculates optimal set of tiles to render and updates quad tree <nodes>
	UpdateTilesToRenderParams params{ frustum, cameraPos, renderDebug, resultOut };
	BuildQuadTree(QuadTreeNodeId{}, params);

	size_t oldNodeCount = nodes.GetCount();

	// Next we need to update the quad tree so that it forms a restricted quad tree
	RestrictQuadTree();

	//KK_LOG_DEBUG("Nodes, original: {}, after restriction: {}", oldNodeCount, nodes.GetCount());

	// Then we create the final render tiles from the leaf nodes of the quad tree
	QuadTreeToTiles(0, params);
}

int TerrainQuadTree::BuildQuadTree(const QuadTreeNodeId& id, UpdateTilesToRenderParams& params)
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
	{
		return -1;
	}
	
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
				QuadTreeNodeId tileId{ id.x * 2 + x, id.y * 2 + y, id.level + 1 };

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
	SortedArray<QuadTreeNodeId> parentsToCheck(allocator);
	SortedArray<QuadTreeNodeId> neighborsToCheck(allocator);

	uint8_t currentLevel = maxNodeLevel;
	while (currentLevel > 1)
	{
		for (const auto& node : nodes)
			if (node.id.level == currentLevel)
				parentsToCheck.InsertUnique(GetParentId(node.id));

		// Check
		int tilesPerDim = GetTilesPerDimension(currentLevel - 1);
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
				// Node always has all the children or none of them
				// If currentNode doesn't have children, split
				if (currentNode->children[0] == 0)
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

				// Update currentNode and continue
				int childNodeIndex = currentNode->children[childIndex];

				assert(childNodeIndex != 0);

				currentNode = &nodes[childNodeIndex];
				continue;
			}
		}

		parentsToCheck.Clear();
		neighborsToCheck.Clear();
		currentLevel -= 1;
	}
}

void TerrainQuadTree::QuadTreeToTiles(uint16_t nodeIndex, UpdateTilesToRenderParams& params)
{
	const TerrainQuadTreeNode& node = nodes[nodeIndex];
	if (node.children[0] == 0) // Node has no children
	{
		params.resultOut.PushBack(node.id);
		return;
	}

	for (int y = 0; y < 2; ++y)
		for (int x = 0; x < 2; ++x)
			QuadTreeToTiles(node.children[y * 2 + x], params);
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

int TerrainQuadTree::GetTilesPerDimension(uint8_t level)
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
	constexpr int texResolution = TerrainTile::ResolutionWithBorder;
	float tileRes = static_cast<float>(TerrainTile::Resolution);

	for (int pixY = 0; pixY < texResolution; ++pixY)
	{
		for (int pixX = 0; pixX < texResolution; ++pixX)
		{
			float cx = (pixX / tileRes + tileX) * tileScale;
			float cy = (pixY / tileRes + tileY) * tileScale;

			size_t pixelIndex = pixY * texResolution + pixX;
			tile.heightData[pixelIndex] = TestData(cx, cy);
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

}
