#include "Graphics/TerrainQuadTree.hpp"

#include "doctest/doctest.h"

#include "Memory/Allocator.hpp"

#include "Rendering/RenderDevice.hpp"

namespace kokko
{

TerrainQuadTree::TerrainQuadTree(Allocator* allocator, RenderDevice* renderDevice) :
	allocator(allocator),
	renderDevice(renderDevice),
	tiles(nullptr),
	tileTextureIds(nullptr),
	treeLevels(0),
	tileCount(0)
{
}

TerrainQuadTree::~TerrainQuadTree()
{
	allocator->Deallocate(tileTextureIds);
	allocator->Deallocate(tiles);
}

void TerrainQuadTree::Initialize(int levels)
{
	treeLevels = levels;
	tileCount = GetTileCountForLevelCount(levels);

	float terrainSize = 1024.0f;
	float tileRes = static_cast<float>(TerrainTile::Resolution);
	int tileResi = static_cast<int>(TerrainTile::Resolution);

	void* buffer = allocator->Allocate(tileCount * sizeof(TerrainTile));
	tiles = static_cast<TerrainTile*>(buffer);

	for (int levelIdx = 0; levelIdx < treeLevels; ++levelIdx)
	{
		int tilesPerDimension = GetTilesPerDimension(levelIdx);

		for (int tileY = 0; tileY < tilesPerDimension; ++tileY)
		{
			for (int tileX = 0; tileX < tilesPerDimension; ++tileX)
			{
				int tileIdx = GetTileIndex(levelIdx, tileX, tileY);
				TerrainTile& tile = tiles[tileIdx];

				for (int pixY = 0; pixY < TerrainTile::Resolution; ++pixY)
				{
					for (int pixX = 0; pixX < TerrainTile::Resolution; ++pixX)
					{
						float tileScale = terrainSize / tilesPerDimension;
						float cx = (pixX / tileRes) * tileScale;
						float cy = (pixY / tileRes) * tileScale;

						float normalized = 0.5f + std::sin(cx * 0.031415f) * 0.25f + std::sin(cy * 0.031415f) * 0.25f;
						uint16_t height = static_cast<uint16_t>(normalized * UINT16_MAX);

						size_t pixelIndex = pixY * TerrainTile::Resolution + pixX;
						tile.heightData[pixelIndex] = height;
					}
				}
			}
		}
	}

	tileTextureIds = static_cast<uint32_t*>(allocator->Allocate(tileCount * sizeof(uint32_t)));
	renderDevice->CreateTextures(tileCount, tileTextureIds);

	for (int levelIdx = 0; levelIdx < treeLevels; ++levelIdx)
	{
		int tilesPerDimension = GetTilesPerDimension(levelIdx);

		for (int tileY = 0; tileY < tilesPerDimension; ++tileY)
		{
			for (int tileX = 0; tileX < tilesPerDimension; ++tileX)
			{
				int tileIdx = GetTileIndex(levelIdx, tileX, tileY);
				
				renderDevice->BindTexture(RenderTextureTarget::Texture2d, tileTextureIds[tileIdx]);

				RenderCommandData::SetTextureStorage2D textureStorage{
					RenderTextureTarget::Texture2d, 1, RenderTextureSizedFormat::R16,
					tileResi, tileResi
				};
				renderDevice->SetTextureStorage2D(&textureStorage);

				RenderCommandData::SetTextureSubImage2D subimage{
					RenderTextureTarget::Texture2d, 0, 0, 0, tileResi, tileResi, RenderTextureBaseFormat::R,
					RenderTextureDataType::UnsignedShort, tiles[tileIdx].heightData
				};
				renderDevice->SetTextureSubImage2D(&subimage);
			}
		}
	}
}

int TerrainQuadTree::GetTilesPerDimension(int level)
{
	int levelSize = 1;

	for (int i = 0; i < level; ++i)
		levelSize *= 2;

	return levelSize;
}

TEST_CASE("TerrainQuadTree.GetTilesPerDimension")
{
	CHECK(TerrainQuadTree::GetTilesPerDimension(0) == 1);
	CHECK(TerrainQuadTree::GetTilesPerDimension(1) == 2);
	CHECK(TerrainQuadTree::GetTilesPerDimension(2) == 4);
	CHECK(TerrainQuadTree::GetTilesPerDimension(3) == 8);
}

int TerrainQuadTree::GetTileIndex(int level, int x, int y)
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

int TerrainQuadTree::GetTileCountForLevelCount(int levelCount)
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

}
