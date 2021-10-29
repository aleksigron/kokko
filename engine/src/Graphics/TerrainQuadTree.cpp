#include "Graphics/TerrainQuadTree.hpp"

#include <cassert>

#include "doctest/doctest.h"

#include "Debug/Debug.hpp"
#include "Debug/DebugTextRenderer.hpp"
#include "Debug/DebugVectorRenderer.hpp"

#include "Graphics/TerrainSystem.hpp"

#include "Math/BoundingBox.hpp"
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

void TerrainQuadTree::CreateResources(Allocator* allocator, RenderDevice* renderDevice, int levels,
	const TerrainParameters& params)
{
	constexpr int tileResolution = TerrainTile::Resolution;
	constexpr int texResolution = TerrainTile::ResolutionWithBorder;

	treeLevels = levels;
	tileCount = GetTileCountForLevelCount(levels);

	terrainWidth = params.terrainSize;
	terrainBottom = params.heightOrigin;
	terrainHeight = params.heightRange;

	void* buffer = allocator->Allocate(tileCount * sizeof(TerrainTile));
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

void TerrainQuadTree::GetTilesToRender(
	const FrustumPlanes& frustum, const Mat4x4f& viewProj, Array<TerrainTileId>& resultOut)
{
	RenderTile(TerrainTileId{}, frustum, viewProj, resultOut);
}

void TerrainQuadTree::RenderTile(
	const TerrainTileId& id,
	const FrustumPlanes& frustum,
	const Mat4x4f& vp,
	Array<TerrainTileId>& resultOut)
{
	float tileScale = GetTileScale(id.level);

	float halfTerrainSize = terrainWidth * 0.5f;
	Vec3f levelOrigin(-halfTerrainSize, 0.0f, -halfTerrainSize);
	float tileWidth = terrainWidth * tileScale;
	Vec3f tileMin = levelOrigin + Vec3f(id.x * tileWidth, terrainBottom, id.y * tileWidth);
	Vec3f tileSize(tileWidth, terrainHeight, tileWidth);

	BoundingBox tileBounds;
	tileBounds.extents = tileSize * 0.5f;
	tileBounds.center = tileMin + tileBounds.extents;

	if (Intersect::FrustumAabb(frustum, tileBounds) == false)
	{
		return;
	}

	if (id.level + 1 == treeLevels)
	{
		resultOut.PushBack(id);
		return;
	}
	
	constexpr float maximumSize = 0.5f;

	static const Vec3f boxCorners[] = {
		Vec3f(-1.0f, 0.0f, -1.0f),
		Vec3f(1.0f, 0.0f, -1.0f),
		Vec3f(-1.0f, 0.0f, 1.0f),
		Vec3f(1.0f, 0.0f, 1.0f)
	};

	Vec2f min(1e9f, 1e9f);
	Vec2f max(-1e9f, -1e9f);

	Vec3f screenCoord;

	for (unsigned cornerIdx = 0; cornerIdx < 4; ++cornerIdx)
	{
		Vec3f corner = Vec3f::Hadamard(tileBounds.extents, boxCorners[cornerIdx]);

		Vec4f proj = vp * Vec4f(tileBounds.center + corner, 1.0f);
		screenCoord = proj.xyz() * (1.0f / proj.w) * 0.5f + Vec3f(0.5f, 0.5f, 0.0f);

		min.x = std::min(screenCoord.x, min.x);
		min.y = std::min(screenCoord.y, min.y);
		max.x = std::max(screenCoord.x, max.x);
		max.y = std::max(screenCoord.y, max.y);
	}

	Vec2f size(max.x - min.x, max.y - min.y);
	

	//float depth = Vec3f::Dot(center - eyePos, eyeForward);

	float sizeMaxAxis = std::max(size.x, size.y);
	bool tileIsSmallEnough = sizeMaxAxis < maximumSize;

	auto vr = Debug::Get()->GetVectorRenderer();
	auto tr = Debug::Get()->GetTextRenderer();
	Color col(1.0f, 0.0f, 1.0f);

	if (tileIsSmallEnough)
	{
		resultOut.PushBack(id);

		Vec3f translate;
		translate.x = -halfTerrainSize + tileWidth * (id.x + 0.5f);
		translate.y = 0.0f;
		translate.z = -halfTerrainSize + tileWidth * (id.y + 0.5f);

		Vec3f scale(tileWidth, 0.0f, tileWidth);

		Mat4x4f transform = Mat4x4f::Translate(translate) * Mat4x4f::Scale(scale);

		vr->DrawWireCube(transform, col);

		char buf[32];
		auto [out, size] = fmt::format_to_n(buf, sizeof(buf), "{:.1f},{:.1f}", tileBounds.center.x, tileBounds.center.z);

		tr->AddTextNormalized(StringRef(buf, size), screenCoord.xy());
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

				RenderTile(tileId, frustum, vp, resultOut);
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
	float f = 0.1f;
	float a = 0.12f;

	float sum = 0.5f;
	for (int i = 1; i <= 6; i += 5)
	{
		sum += std::sin(x * f * i) * a / i + std::sin(y * f * i) * a / i;
	}
	return static_cast<uint16_t>(sum * UINT16_MAX);
}

}
