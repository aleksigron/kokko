#include "Graphics/TerrainQuadTree.hpp"

#include <cassert>

#include "doctest/doctest.h"

#include "Graphics/TerrainSystem.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/CameraParameters.hpp"
#include "Rendering/RenderDevice.hpp"

namespace kokko
{

TerrainQuadTree::TerrainQuadTree() :
	tiles(nullptr),
	tileTextureIds(nullptr),
	treeLevels(0),
	tileCount(0)
{
}

void TerrainQuadTree::CreateResources(Allocator* allocator, RenderDevice* renderDevice, int levels)
{
	constexpr int tileResolution = TerrainTile::Resolution;
	constexpr int texResolution = TerrainTile::ResolutionWithBorder;

	treeLevels = levels;
	tileCount = GetTileCountForLevelCount(levels);

	float terrainSize = 1024.0f;

	void* buffer = allocator->Allocate(tileCount * sizeof(TerrainTile));
	tiles = static_cast<TerrainTile*>(buffer);

	for (int levelIdx = 0; levelIdx < treeLevels; ++levelIdx)
	{
		int tilesPerDimension = GetTilesPerDimension(levelIdx);

		for (int tileY = 0; tileY < tilesPerDimension; ++tileY)
		{
			for (int tileX = 0; tileX < tilesPerDimension; ++tileX)
			{
				float tileScale = terrainSize / tilesPerDimension;
				int tileIdx = GetTileIndex(levelIdx, tileX, tileY);
				TerrainTile& tile = tiles[tileIdx];

				CreateTileTestData(tile, tileX, tileY, tileScale);
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
					texResolution, texResolution
				};
				renderDevice->SetTextureStorage2D(&textureStorage);

				RenderCommandData::SetTextureSubImage2D subimage{
					RenderTextureTarget::Texture2d, 0, 0, 0,
					texResolution, texResolution, RenderTextureBaseFormat::R,
					RenderTextureDataType::UnsignedShort, tiles[tileIdx].heightData
				};
				renderDevice->SetTextureSubImage2D(&subimage);
			}
		}
	}
}

void TerrainQuadTree::DestroyResources(Allocator* allocator, RenderDevice* renderDevice)
{
	if (tileTextureIds != nullptr)
		renderDevice->DestroyTextures(static_cast<unsigned int>(tileCount), tileTextureIds);

	allocator->Deallocate(tiles);
	allocator->Deallocate(tileTextureIds);
}

void TerrainQuadTree::GetTilesToRender(const CameraParameters& camera, Array<TerrainTileId>& resultOut)
{
	int currentLevel = treeLevels - 1;
	int tilesPerDimension = GetTilesPerDimension(currentLevel);
	resultOut.Reserve(tilesPerDimension * tilesPerDimension);
	for (int y = 0; y < tilesPerDimension; ++y)
	{
		for (int x = 0; x < tilesPerDimension; ++x)
		{
			TerrainTileId& tile = resultOut.PushBack();

			tile.level = currentLevel;
			tile.x = x;
			tile.y = y;
		}
	}
}

int TerrainQuadTree::GetLevelCount() const
{
	return treeLevels;
}

const TerrainTile* TerrainQuadTree::GetTile(int level, int x, int y)
{
	return &tiles[GetTileIndex(level, x, y)];
}

unsigned int TerrainQuadTree::GetTileHeightTexture(int level, int x, int y)
{
	return tileTextureIds[GetTileIndex(level, x, y)];
}

int TerrainQuadTree::GetTilesPerDimension(int level)
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

float TerrainQuadTree::GetTileScale(int level)
{
	return std::pow(2.0f, static_cast<float>(-level));
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
	float f = 0.01f;
	float a = 0.12f;

	float sum = 0.5f;
	for (int i = 1; i <= 6; i += 5)
	{
		sum += std::sin(x * f * i) * a / i + std::sin(y * f * i) * a / i;
	}
	return static_cast<uint16_t>(sum * UINT16_MAX);
}

}
