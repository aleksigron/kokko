#include "Graphics/TerrainQuadTree.hpp"

#include <cassert>

#include "doctest/doctest.h"

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

TerrainQuadTree::TerrainQuadTree() :
	tiles(nullptr),
	tileTextureIds(nullptr),
	treeLevels(0),
	tileCount(0),
	terrainWidth(0.0f),
	terrainBottom(0.0f),
	terrainHeight(0.0f)
{
}

void TerrainQuadTree::CreateResources(Allocator* allocator, kokko::render::Device* renderDevice, int levels,
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

	for (int levelIdx = 0; levelIdx < treeLevels; ++levelIdx)
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

void TerrainQuadTree::DestroyResources(Allocator* allocator, kokko::render::Device* renderDevice)
{
	if (tileTextureIds != nullptr)
		renderDevice->DestroyTextures(static_cast<unsigned int>(tileCount), tileTextureIds);

	allocator->Deallocate(tiles);
	allocator->Deallocate(tileTextureIds);
}

void TerrainQuadTree::GetTilesToRender(const FrustumPlanes& frustum, const Vec3f& cameraPos,
	const RenderDebugSettings& renderDebug, Array<TerrainTileId>& resultOut)
{
	KOKKO_PROFILE_SCOPE("TerrainQuadTree::GetTilesToRender()");
	GetRenderTilesParams params{ frustum, cameraPos, renderDebug, resultOut };
	RenderTile(TerrainTileId{}, params);
}

void TerrainQuadTree::RenderTile(const TerrainTileId& id, GetRenderTilesParams& params)
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
		return;
	}

	if (id.level + 1 == treeLevels)
	{
		params.resultOut.PushBack(id);

		return;
	}
	
	constexpr float sizeFactor = 0.5f;

	float distance = (tileBounds.center - params.cameraPos).Magnitude();
	bool tileIsSmallEnough = tileWidth < distance * sizeFactor;

	auto vr = Debug::Get()->GetVectorRenderer();
	auto tr = Debug::Get()->GetTextRenderer();
	Color col(1.0f, 0.0f, 1.0f);

	if (tileIsSmallEnough)
	{
		params.resultOut.PushBack(id);

		if (params.renderDebug.IsFeatureEnabled(RenderDebugFeatureFlag::DrawTerrainTiles))
		{
			Vec3f scale = tileSize;
			scale.y = 0.0f;

			Mat4x4f transform = Mat4x4f::Translate(tileBounds.center) * Mat4x4f::Scale(scale);

			vr->DrawWireCube(transform, col);
		}
	}
	else
	{
		for (int y = 0; y < 2; ++y)
		{
			for (int x = 0; x < 2; ++x)
			{
				TerrainTileId tileId;
				tileId.level = id.level + 1;
				tileId.x = id.x * 2 + x;
				tileId.y = id.y * 2 + y;

				RenderTile(tileId, params);
			}
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

render::TextureId TerrainQuadTree::GetTileHeightTexture(int level, int x, int y)
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

}
