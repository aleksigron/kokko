#include "Graphics/TerrainSystem.hpp"

#include <cassert>
#include <cmath>
#include <cstring>

#include "stb_image/stb_image.h"

#include "Core/Core.hpp"

#include "Engine/Entity.hpp"

#include "Graphics/TerrainQuadTree.hpp"
#include "Graphics/GraphicsFeatureCommandList.hpp"

#include "Math/Mat4x4.hpp"
#include "Math/Vec3.hpp"
#include "Math/Vec2.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/CommandEncoder.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/Renderer.hpp"
#include "Rendering/RenderViewport.hpp"
#include "Rendering/StaticUniformBuffer.hpp"
#include "Rendering/Uniform.hpp"
#include "Rendering/VertexFormat.hpp"

#include "Resources/AssetLoader.hpp"
#include "Resources/ShaderManager.hpp"
#include "Resources/TextureManager.hpp"

namespace kokko
{

const TerrainId TerrainId::Null = TerrainId{ 0 };

struct TerrainUniformBlock
{
	static const unsigned int BindingPoint = 2;

	alignas(16) Mat4x4f MVP;
	alignas(16) Mat4x4f MV;

	alignas(8) Vec2f textureScale;

	alignas(8) Vec2f tileOffset;
	alignas(4) float tileScale;

	alignas(4) float terrainSize;
	alignas(4) float terrainSideVerts;
	alignas(4) float heightOrigin;
	alignas(4) float heightRange;

	alignas(4) float metalness;
	alignas(4) float roughness;
};

TerrainSystem::TerrainInstance::TerrainInstance(TerrainQuadTree&& quadTree) :
	quadTree(std::move(quadTree))
{
}

TerrainSystem::TerrainInstance& TerrainSystem::TerrainInstance::operator=(TerrainInstance&& other) noexcept
{
	entity = other.entity;
	textureScale = other.textureScale;
	hasHeightTextureUid = other.hasHeightTextureUid;
	heightTextureUid = other.heightTextureUid;
	heightmapPixels = other.heightmapPixels;
	heightmapSize = other.heightmapSize;
	albedoTexture = other.albedoTexture;
	roughnessTexture = other.roughnessTexture;
	roughnessValue = other.roughnessValue;
	quadTree = std::move(other.quadTree);

	other.entity = Entity::Null;
	other.textureScale = Vec2f();
	other.hasHeightTextureUid = false;
	other.heightmapPixels = nullptr;
	other.heightmapSize = 0;
	other.albedoTexture = TextureInfo();
	other.roughnessTexture = TextureInfo();
	other.roughnessValue = 1.0f;

	return *this;
}

TerrainSystem::TerrainSystem(
	Allocator* allocator,
	AssetLoader* assetLoader,
	render::Device* renderDevice,
	ShaderManager* shaderManager,
	TextureManager* textureManager) :
	allocator(allocator),
	assetLoader(assetLoader),
	renderDevice(renderDevice),
	shaderManager(shaderManager),
	textureManager(textureManager),
	uniformBlockStride(0),
	uniformBlocksAllocated(0),
	uniformBlocksRendered(0),
	uniformBufferId(0),
	terrainShader(ShaderId::Null),
	terrainDepthShader(ShaderId::Null),
	entityMap(allocator),
	vertexData(VertexData{}),
	textureSampler(0),
	uniformStagingBuffer(allocator),
	instances(allocator)
{
	instances.Reserve(16);
	instances.PushBack(); // Reserve index 0 as Null
}

TerrainSystem::~TerrainSystem()
{
	RemoveAll();

	if (vertexData.indexBuffers[0] != 0)
		renderDevice->DestroyBuffers(MeshTypeCount, vertexData.indexBuffers);

	if (vertexData.vertexBuffer != 0)
		renderDevice->DestroyBuffers(1, &vertexData.vertexBuffer);

	if (vertexData.vertexArrays[0] != 0)
		renderDevice->DestroyVertexArrays(MeshTypeCount, vertexData.vertexArrays);

	if (uniformBufferId != 0)
		renderDevice->DestroyBuffers(1, &uniformBufferId);

	if (textureSampler != 0)
		renderDevice->DestroySamplers(1, &textureSampler);
}

void TerrainSystem::Initialize()
{
	KOKKO_PROFILE_FUNCTION();

	auto scope = renderDevice->CreateDebugScope(0, ConstStringView("TerrainSys_InitResources"));

	// Get uniform buffer alignment

	int aligment = 0;
	renderDevice->GetIntegerValue(RenderDeviceParameter::UniformBufferOffsetAlignment, &aligment);

	uniformBlockStride = Math::RoundUpToMultiple(sizeof(TerrainUniformBlock), static_cast<size_t>(aligment));

	// Shaders

	terrainShader = shaderManager->FindShaderByPath(
		ConstStringView("engine/shaders/deferred_geometry/terrain.glsl"));
	assert(terrainShader != ShaderId::Null);

	terrainDepthShader = shaderManager->FindShaderByPath(
		ConstStringView("engine/shaders/deferred_geometry/terrain_depth.glsl"));
	assert(terrainDepthShader != ShaderId::Null);

	// Sampler

	RenderSamplerParameters samplerParams{
		RenderTextureFilterMode::Nearest, RenderTextureFilterMode::Nearest,
		RenderTextureWrapMode::ClampToEdge, RenderTextureWrapMode::ClampToEdge,
		RenderTextureWrapMode::ClampToEdge, RenderTextureCompareMode::None,
		RenderDepthCompareFunc::Always
	};
	renderDevice->CreateSamplers(1, &samplerParams, &textureSampler);

	// Vertex data

	CreateVertexAndIndexData();
}

TerrainId TerrainSystem::Lookup(Entity e)
{
	auto* pair = entityMap.Lookup(e.id);
	return pair != nullptr ? pair->second : TerrainId{};
}

TerrainId TerrainSystem::AddTerrain(Entity entity)
{
	uint32_t id = static_cast<uint32_t>(instances.GetCount());

	auto mapPair = entityMap.Insert(entity.id);
	mapPair->second.i = id;

	instances.EmplaceBack(TerrainQuadTree(allocator, renderDevice));
	TerrainInstance& instance = instances.GetBack();
	instance.entity = entity;

	return TerrainId{ id };
}

void TerrainSystem::RemoveTerrain(TerrainId id)
{
	size_t count = instances.GetCount();

	assert(id != TerrainId::Null);
	assert(id.i < count);

	TerrainInstance& instance = instances[id.i];

	// Remove from entity map
	auto* pair = entityMap.Lookup(instance.entity.id);
	if (pair != nullptr)
		entityMap.Remove(pair);

	if (count > 2 && id.i + 1 < count) // We need to move another object
	{
		size_t moveIdx = count - 1;

		// Update the swapped object's id in the entity map
		auto* moveKv = entityMap.Lookup(instances[moveIdx].entity.id);
		if (moveKv != nullptr)
			moveKv->second = id;

		instance = std::move(instances[moveIdx]);
	}

	instances.PopBack();
}

void TerrainSystem::RemoveAll()
{
	instances.Resize(1);
	entityMap.Clear();
}

Optional<Uid> TerrainSystem::GetHeightTexture(TerrainId id) const
{
	assert(id.i != 0 && id.i < instances.GetCount());

	if (instances[id.i].hasHeightTextureUid == false)
		return Optional<Uid>();

	return instances[id.i].heightTextureUid;
}

void TerrainSystem::SetHeightTexture(TerrainId id, Uid textureUid)
{
	assert(id.i != 0 && id.i < instances.GetCount());

	if (instances[id.i].hasHeightTextureUid == false || instances[id.i].heightTextureUid != textureUid)
	{
		if (LoadHeightmap(id, textureUid))
		{
			instances[id.i].heightTextureUid = textureUid;
			instances[id.i].hasHeightTextureUid = true;
		}
		else
			KK_LOG_ERROR("Terrain height texture was not set, because loading failed");
	}
}

TextureId TerrainSystem::GetAlbedoTextureId(TerrainId id) const
{
	assert(id.i != 0 && id.i < instances.GetCount());

	return instances[id.i].albedoTexture.id;
}

void TerrainSystem::SetAlbedoTexture(TerrainId id, TextureId textureId, render::TextureId textureObject)
{
	assert(id.i != 0 && id.i < instances.GetCount());

	instances[id.i].albedoTexture.id = textureId;
	instances[id.i].albedoTexture.renderId = textureObject;
}

TextureId TerrainSystem::GetRoughnessTextureId(TerrainId id) const
{
	assert(id.i != 0 && id.i < instances.GetCount());

	return instances[id.i].roughnessTexture.id;
}

void TerrainSystem::SetRoughnessTexture(TerrainId id, TextureId textureId, render::TextureId textureObject)
{
	assert(id.i != 0 && id.i < instances.GetCount());

	instances[id.i].roughnessTexture.id = textureId;
	instances[id.i].roughnessTexture.renderId = textureObject;
}

float TerrainSystem::GetRoughnessValue(TerrainId id) const
{
	return instances[id.i].roughnessValue;
}

void TerrainSystem::SetRoughnessValue(TerrainId id, float roughness)
{
	assert(id.i != 0 && id.i < instances.GetCount());

	instances[id.i].roughnessValue = roughness;
}

void TerrainSystem::Upload(const UploadParameters& params)
{
	uniformBlocksRendered = 0;
	const bool drawShadows = params.renderDebug.IsFeatureEnabled(RenderDebugFeatureFlag::ExperimentalTerrainShadows);

	for (size_t i = 1, count = instances.GetCount(); i < count; ++i)
	{
		// FIXME: Rendering will not be correct if multiple terrain instances exist in the world
		// All uniform block info needs to be made instance-specific

		const RenderViewport& fullscreenViewport = params.viewports[params.fullscreenViewportIndex];

		// Select tiles to render

		TerrainInstance& instance = instances[i];
		TerrainQuadTree& quadTree = instance.quadTree;
		quadTree.UpdateTilesToRender(fullscreenViewport.frustum, fullscreenViewport.position, params.renderDebug);
		ArrayView<const TerrainTileDrawInfo> tiles = quadTree.GetTilesToRender();
		uint32_t tileCount = static_cast<uint32_t>(tiles.GetCount());

		// Update uniform buffer

		float terrainWidth = quadTree.GetSize();
		float terrainHeight = quadTree.GetHeight();

		TerrainUniformBlock uniforms;
		uniforms.textureScale = instance.textureScale;
		uniforms.tileOffset = Vec2f();
		uniforms.tileScale = 1.0f;
		uniforms.terrainSize = terrainWidth;
		uniforms.terrainSideVerts = static_cast<float>(TerrainTile::VerticesPerSide);
		uniforms.heightOrigin = quadTree.GetBottom();
		uniforms.heightRange = terrainHeight;
		uniforms.metalness = 0.0f;
		uniforms.roughness = instance.roughnessValue;

		int shadowViewports = drawShadows ? params.shadowViewportsEndIndex - params.shadowViewportsBeginIndex : 0;
		int viewportCount = shadowViewports + 1;
		uint32_t uniformBlockCount = tileCount * viewportCount;
		uniformStagingBuffer.Resize(uniformBlockCount * uniformBlockStride);

		if (uniformBlockCount > uniformBlocksAllocated)
		{
			uint32_t newAllocated = Math::RoundUpToMultiple(uniformBlockCount * 4 / 3, 16u);
			uint32_t newByteCount = newAllocated * uniformBlockStride;

			if (uniformBufferId != render::BufferId::Null)
				renderDevice->DestroyBuffers(1, &uniformBufferId);

			renderDevice->CreateBuffers(1, &uniformBufferId);
			renderDevice->SetBufferStorage(uniformBufferId, newByteCount, nullptr, BufferStorageFlags::Dynamic);

			uniformBlocksAllocated = newAllocated;
		}

		{
			KOKKO_PROFILE_SCOPE("Setup tile uniform data");

			auto updateTileInfo = [&uniforms](const TerrainTileDrawInfo& tile)
			{
				const float halfTileCount = 0.5f * TerrainQuadTree::GetTilesPerDimension(tile.id.level);
				const Vec2f levelOrigin(-halfTileCount, -halfTileCount);

				uniforms.tileOffset = levelOrigin + Vec2f(static_cast<float>(tile.id.x), static_cast<float>(tile.id.y));
				uniforms.tileScale = TerrainQuadTree::GetTileScale(tile.id.level);
			};

			uint32_t blocksWritten = 0;

			auto writeToBuffer = [this, &uniforms, &blocksWritten]()
			{
				uint8_t* dest = &uniformStagingBuffer[blocksWritten * uniformBlockStride];
				std::memcpy(dest, &uniforms, sizeof(uniforms));

				blocksWritten += 1;
			};

			// Shadow viewports
			if (drawShadows)
			{
				uint32_t vpEnd = params.shadowViewportsEndIndex;
				for (uint32_t vpIdx = params.shadowViewportsBeginIndex; vpIdx != vpEnd; ++vpIdx)
				{
					uniforms.MVP = params.viewports[vpIdx].viewProjection;
					uniforms.MV = params.viewports[vpIdx].view.inverse;

					for (const auto& tile : tiles)
					{
						updateTileInfo(tile);
						writeToBuffer();
					}
				}
			}

			uniforms.MVP = fullscreenViewport.viewProjection;
			uniforms.MV = fullscreenViewport.view.inverse;

			// Fullscreen viewport
			for (const auto& tile : tiles)
			{
				updateTileInfo(tile);
				writeToBuffer();
			}
		}

		uint32_t updateBytes = uniformBlockCount * uniformBlockStride;

		renderDevice->SetBufferSubData(uniformBufferId, 0, updateBytes, uniformStagingBuffer.GetData());
	}
}

void TerrainSystem::Submit(const SubmitParameters& params)
{
	float depth = 0.0f; // Render (mostly) first

	for (size_t i = 1, count = instances.GetCount(); i < count; ++i)
	{
		params.commandList->AddToViewport(params.fullscreenViewportIndex, RenderPassType::OpaqueGeometry, depth, i);

		if (params.renderDebug.IsFeatureEnabled(RenderDebugFeatureFlag::ExperimentalTerrainShadows))
		{
			const size_t vpEnd = params.shadowViewportsEndIndex;
			for (size_t vpIdx = params.shadowViewportsBeginIndex; vpIdx != vpEnd; ++vpIdx)
			{
				params.commandList->AddToViewport(vpIdx, RenderPassType::OpaqueGeometry, depth, i);
			}
		}
	}
}

void TerrainSystem::Render(const RenderParameters& parameters)
{
	KOKKO_PROFILE_SCOPE("TerrainSystem::Render");

	TerrainInstance& instance = instances[parameters.featureObjectId];
	TerrainQuadTree& quadTree = instance.quadTree;
	ArrayView<const TerrainTileDrawInfo> tiles = quadTree.GetTilesToRender();

	render::CommandEncoder* encoder = parameters.encoder;

	const bool isFullscreenViewport = parameters.renderingViewportIndex == parameters.fullscreenViewportIndex;
	ShaderId shaderId = isFullscreenViewport ? terrainShader : terrainDepthShader;
	const ShaderData& shader = shaderManager->GetShaderData(shaderId);
	encoder->UseShaderProgram(shader.driverId);
	encoder->BindSampler(0, textureSampler); // For height texture

	const TextureUniform* heightMap = shader.uniforms.FindTextureUniformByNameHash("height_map"_hash);

	if (isFullscreenViewport)
	{
		const TextureUniform* albedoMap = shader.uniforms.FindTextureUniformByNameHash("albedo_map"_hash);
		const TextureUniform* roughnessMap = shader.uniforms.FindTextureUniformByNameHash("roughness_map"_hash);

		if (albedoMap != nullptr)
		{
			kokko::render::TextureId textureObjectId;
			if (instance.albedoTexture.renderId != 0)
				textureObjectId = instance.albedoTexture.renderId;
			else
			{
				TextureId emptyAlbedoId = textureManager->GetId_White2D();
				textureObjectId = textureManager->GetTextureData(emptyAlbedoId).textureObjectId;
			}

			encoder->BindTextureToShader(albedoMap->uniformLocation, 1, textureObjectId);
		}

		if (roughnessMap != nullptr)
		{
			kokko::render::TextureId textureObjectId;
			if (instance.roughnessTexture.renderId != 0)
				textureObjectId = instance.roughnessTexture.renderId;
			else
			{
				TextureId emptyRoughnessId = textureManager->GetId_White2D();
				textureObjectId = textureManager->GetTextureData(emptyRoughnessId).textureObjectId;
			}

			encoder->BindTextureToShader(roughnessMap->uniformLocation, 2, textureObjectId);
		}
	}

	// TODO: Cull tiles to render in shadow viewports

	{
		KOKKO_PROFILE_SCOPE("Render tiles");

		uint8_t prevMeshType = 255;
		for (const auto& tile : tiles)
		{
			uint8_t tileMeshTypeIndex = static_cast<uint8_t>(tile.edgeType);
			if (tileMeshTypeIndex >= MeshTypeCount)
				continue;

			if (tileMeshTypeIndex != prevMeshType)
			{
				encoder->BindVertexArray(vertexData.vertexArrays[tileMeshTypeIndex]);
				prevMeshType = tileMeshTypeIndex;
			}

			intptr_t rangeOffset = uniformBlocksRendered * uniformBlockStride;
			uniformBlocksRendered += 1;

			encoder->BindBufferRange(RenderBufferTarget::UniformBuffer, UniformBlockBinding::Object, uniformBufferId,
				rangeOffset, uniformBlockStride);

			if (heightMap != nullptr)
			{
				render::TextureId heightTex = quadTree.GetTileHeightTexture(tile.id);

				encoder->BindTextureToShader(heightMap->uniformLocation, 0, heightTex);
			}

			int count = vertexData.indexCounts[tileMeshTypeIndex];
			encoder->DrawIndexed(RenderPrimitiveMode::Triangles, RenderIndexType::UnsignedShort, count, 0, 0);
		}
	}
}

bool TerrainSystem::LoadHeightmap(TerrainId id, Uid textureUid)
{
	Array<uint8_t> buffer(allocator);
	AssetLoader::LoadResult loadResult = assetLoader->LoadAsset(textureUid, buffer);
	if (loadResult.success == false)
		return false;

	assert(loadResult.assetType == AssetType::Texture);

	TextureAssetMetadata metadata;
	if (loadResult.metadataSize == sizeof(metadata))
		memcpy(&metadata, buffer.GetData(), loadResult.metadataSize);

	auto assetView = buffer.GetSubView(loadResult.assetStart, loadResult.assetStart + loadResult.assetSize);

	stbi_set_flip_vertically_on_load(true);
	int width, height, nrComponents;
	uint16_t* imageBytes = nullptr;

	{
		KOKKO_PROFILE_SCOPE("stbi_load_16_from_memory()");

		const uint8_t* fileBytesPtr = assetView.GetData();
		int length = static_cast<int>(assetView.GetCount());
		imageBytes = stbi_load_16_from_memory(fileBytesPtr, length, &width, &height, &nrComponents, 1);
	}

	if (imageBytes == nullptr)
	{
		KK_LOG_ERROR("Terrain heightmap loading failed: image loading failed");
		return false;
	}

	if (width != height)
	{
		KK_LOG_ERROR("Terrain heightmap loading failed: image isn't square");
		return false;
	}

	TerrainInstance& instance = instances[id.i];

	if (instance.heightmapPixels != nullptr)
		stbi_image_free(instance.heightmapPixels);

	instance.heightmapPixels = imageBytes;
	instance.heightmapSize = width;

	instance.quadTree.SetHeightmap(imageBytes, width);

	return true;
}

void TerrainSystem::CreateVertexAndIndexData()
{
	// Create vertex data

	constexpr uint32_t sideVerts = TerrainTile::VerticesPerSide;
	uint32_t vertCount = sideVerts * sideVerts;
	size_t vertexComponents = 2;
	size_t vertSize = sizeof(float) * vertexComponents;
	uint32_t vertBytes = static_cast<uint32_t>(vertSize * vertCount);
	float* vertexBuf = static_cast<float*>(allocator->Allocate(vertBytes, "TerrainSystem.CreateVertexData() vertexBuf"));

	constexpr int sideQuads = TerrainTile::QuadsPerSide;
	uint32_t quadIndices = 3 * 2; // 3 indices per triangle, 2 triangles per quad
	uint32_t maxIndexCount = sideQuads * sideQuads * quadIndices;
	uint32_t maxIndexBytes = maxIndexCount * sizeof(uint16_t);
	uint16_t* indexBuf = static_cast<uint16_t*>(allocator->Allocate(maxIndexBytes, "TerrainSystem.CreateVertexData() indexBuf"));

	float quadSize = 1.0f / sideQuads;

	renderDevice->CreateVertexArrays(MeshTypeCount, vertexData.vertexArrays);
	renderDevice->CreateBuffers(1, &vertexData.vertexBuffer);
	renderDevice->CreateBuffers(MeshTypeCount, vertexData.indexBuffers);

	// Set vertex data
	for (size_t y = 0; y < sideVerts; ++y)
	{
		for (size_t x = 0; x < sideVerts; ++x)
		{
			size_t vertIndex = y * sideVerts + x;
			vertexBuf[vertIndex * vertexComponents + 0] = x * quadSize;
			vertexBuf[vertIndex * vertexComponents + 1] = y * quadSize;
		}
	}

	// Upload vertex buffer
	renderDevice->SetBufferStorage(vertexData.vertexBuffer, vertBytes, vertexBuf, BufferStorageFlags::None);

	auto vertexIndex = [](int x, int y) { return static_cast<uint16_t>(y * TerrainTile::VerticesPerSide + x); };
	auto basicQuad = [&indexBuf, &vertexIndex](int x, int y, uint32_t& count)
	{
		indexBuf[count + 0] = vertexIndex(x, y);
		indexBuf[count + 1] = vertexIndex(x, y + 1);
		indexBuf[count + 2] = vertexIndex(x + 1, y);
		indexBuf[count + 3] = vertexIndex(x, y + 1);
		indexBuf[count + 4] = vertexIndex(x + 1, y + 1);
		indexBuf[count + 5] = vertexIndex(x + 1, y);
		count += 6;
	};

	// MeshType_Regular
	{
		uint32_t indexCount = 0;

		for (int y = 0; y < sideQuads; ++y)
			for (int x = 0; x < sideQuads; ++x)
				basicQuad(x, y, indexCount);

		assert(indexCount <= maxIndexCount);
		uint32_t indexBytes = indexCount * sizeof(uint16_t);
		uint8_t meshTypeIndex = static_cast<uint8_t>(TerrainEdgeType::Regular);
		vertexData.indexCounts[meshTypeIndex] = indexCount;
		renderDevice->SetBufferStorage(
			vertexData.indexBuffers[meshTypeIndex], indexBytes, indexBuf, BufferStorageFlags::None);
	}
	
	// MeshType_TopSparse
	{
		uint32_t indexCount = 0;

		for (int y = 0, x = 0; x < sideQuads; x += 2)
		{
			indexBuf[indexCount + 0] = vertexIndex(x, y);
			indexBuf[indexCount + 1] = vertexIndex(x, y + 1);
			indexBuf[indexCount + 2] = vertexIndex(x + 1, y + 1);
			indexBuf[indexCount + 3] = vertexIndex(x, y);
			indexBuf[indexCount + 4] = vertexIndex(x + 1, y + 1);
			indexBuf[indexCount + 5] = vertexIndex(x + 2, y);
			indexBuf[indexCount + 6] = vertexIndex(x + 2, y);
			indexBuf[indexCount + 7] = vertexIndex(x + 1, y + 1);
			indexBuf[indexCount + 8] = vertexIndex(x + 2, y + 1);
			indexCount += 9;
		}

		for (int y = 1; y < sideQuads; ++y)
			for (int x = 0; x < sideQuads; ++x)
				basicQuad(x, y, indexCount);

		assert(indexCount <= maxIndexCount);
		uint32_t indexBytes = indexCount * sizeof(uint16_t);
		uint8_t meshTypeIndex = static_cast<uint8_t>(TerrainEdgeType::TopSparse);
		vertexData.indexCounts[meshTypeIndex] = indexCount;
		renderDevice->SetBufferStorage(
			vertexData.indexBuffers[meshTypeIndex], indexBytes, indexBuf, BufferStorageFlags::None);
	}

	// MeshType_TopRightSparse
	{
		uint32_t indexCount = 0;

		for (int y = 0, x = 0; x < sideQuads; x += 2)
		{
			// +---+
			// |\ /
			// +-+
			indexBuf[indexCount + 0] = vertexIndex(x, y);
			indexBuf[indexCount + 1] = vertexIndex(x, y + 1);
			indexBuf[indexCount + 2] = vertexIndex(x + 1, y + 1);
			indexBuf[indexCount + 3] = vertexIndex(x, y);
			indexBuf[indexCount + 4] = vertexIndex(x + 1, y + 1);
			indexBuf[indexCount + 5] = vertexIndex(x + 2, y);
			indexCount += 6;

			if (x < sideQuads - 2)
			{
				//   +
				//  /|
				// +-+
				indexBuf[indexCount + 0] = vertexIndex(x + 2, y);
				indexBuf[indexCount + 1] = vertexIndex(x + 1, y + 1);
				indexBuf[indexCount + 2] = vertexIndex(x + 2, y + 1);
				indexCount += 3;
			}
			else
			{
				//   +
				//  /|
				// + |
				// |\|
				// +-+
				indexBuf[indexCount + 0] = vertexIndex(x + 2, y);
				indexBuf[indexCount + 1] = vertexIndex(x + 1, y + 1);
				indexBuf[indexCount + 2] = vertexIndex(x + 2, y + 2);
				indexBuf[indexCount + 3] = vertexIndex(x + 2, y + 2);
				indexBuf[indexCount + 4] = vertexIndex(x + 1, y + 1);
				indexBuf[indexCount + 5] = vertexIndex(x + 1, y + 2);
				indexCount += 6;
			}
		}

		for (int y = 1; y < sideQuads; ++y)
		{
			for (int x = 0; x < sideQuads; ++x)
			{
				if (x < sideQuads - 1)
					basicQuad(x, y, indexCount);
				else if ((y & 1) == 0)
				{
					indexBuf[indexCount + 0] = vertexIndex(x, y);
					indexBuf[indexCount + 1] = vertexIndex(x, y + 1);
					indexBuf[indexCount + 2] = vertexIndex(x + 1, y);
					indexBuf[indexCount + 3] = vertexIndex(x + 1, y);
					indexBuf[indexCount + 4] = vertexIndex(x, y + 1);
					indexBuf[indexCount + 5] = vertexIndex(x + 1, y + 2);
					indexBuf[indexCount + 6] = vertexIndex(x + 1, y + 2);
					indexBuf[indexCount + 7] = vertexIndex(x, y + 1);
					indexBuf[indexCount + 8] = vertexIndex(x, y + 2);
					indexCount += 9;
				}
			}
		}

		assert(indexCount <= maxIndexCount);
		uint32_t indexBytes = indexCount * sizeof(uint16_t);
		uint8_t meshTypeIndex = static_cast<uint8_t>(TerrainEdgeType::TopRightSparse);
		vertexData.indexCounts[meshTypeIndex] = indexCount;
		renderDevice->SetBufferStorage(
			vertexData.indexBuffers[meshTypeIndex], indexBytes, indexBuf, BufferStorageFlags::None);
	}

	// MeshType_RightSparse
	{
		uint32_t indexCount = 0;

		for (int y = 0; y < sideQuads; ++y)
		{
			for (int x = 0; x < sideQuads; ++x)
			{
				if (x < sideQuads - 1)
					basicQuad(x, y, indexCount);
				else if ((y & 1) == 0)
				{
					indexBuf[indexCount + 0] = vertexIndex(x, y);
					indexBuf[indexCount + 1] = vertexIndex(x, y + 1);
					indexBuf[indexCount + 2] = vertexIndex(x + 1, y);
					indexBuf[indexCount + 3] = vertexIndex(x + 1, y);
					indexBuf[indexCount + 4] = vertexIndex(x, y + 1);
					indexBuf[indexCount + 5] = vertexIndex(x + 1, y + 2);
					indexBuf[indexCount + 6] = vertexIndex(x + 1, y + 2);
					indexBuf[indexCount + 7] = vertexIndex(x, y + 1);
					indexBuf[indexCount + 8] = vertexIndex(x, y + 2);
					indexCount += 9;
				}
			}
		}

		assert(indexCount <= maxIndexCount);
		uint32_t indexBytes = indexCount * sizeof(uint16_t);
		uint8_t meshTypeIndex = static_cast<uint8_t>(TerrainEdgeType::RightSparse);
		vertexData.indexCounts[meshTypeIndex] = indexCount;
		renderDevice->SetBufferStorage(
			vertexData.indexBuffers[meshTypeIndex], indexBytes, indexBuf, BufferStorageFlags::None);
	}

	// MeshType_RightBottomSparse
	{
		uint32_t indexCount = 0;

		for (int y = 0; y < sideQuads - 1; ++y)
		{
			for (int x = 0; x < sideQuads; ++x)
			{
				if (x < sideQuads - 1)
					basicQuad(x, y, indexCount);
				else if ((y & 1) == 0)
				{
					// +-+
					// |/|
					// + |
					//  \|
					//   +
					indexBuf[indexCount + 0] = vertexIndex(x, y);
					indexBuf[indexCount + 1] = vertexIndex(x, y + 1);
					indexBuf[indexCount + 2] = vertexIndex(x + 1, y);
					indexBuf[indexCount + 3] = vertexIndex(x + 1, y);
					indexBuf[indexCount + 4] = vertexIndex(x, y + 1);
					indexBuf[indexCount + 5] = vertexIndex(x + 1, y + 2);
					indexCount += 6;

					if (y < sideQuads - 2)
					{
						// +
						// |\
						// +-+
						indexBuf[indexCount + 0] = vertexIndex(x + 1, y + 2);
						indexBuf[indexCount + 1] = vertexIndex(x, y + 1);
						indexBuf[indexCount + 2] = vertexIndex(x, y + 2);
						indexCount += 3;
					}
				}
			}
		}

		for (int y = sideQuads - 1, x = 0; x < sideQuads; x += 2)
		{
			if (x < sideQuads - 2)
			{
				indexBuf[indexCount + 0] = vertexIndex(x, y);
				indexBuf[indexCount + 1] = vertexIndex(x, y + 1);
				indexBuf[indexCount + 2] = vertexIndex(x + 1, y);
				indexBuf[indexCount + 3] = vertexIndex(x + 1, y);
				indexBuf[indexCount + 4] = vertexIndex(x, y + 1);
				indexBuf[indexCount + 5] = vertexIndex(x + 2, y + 1);
				indexBuf[indexCount + 6] = vertexIndex(x + 1, y);
				indexBuf[indexCount + 7] = vertexIndex(x + 2, y + 1);
				indexBuf[indexCount + 8] = vertexIndex(x + 2, y);
				indexCount += 9;
			}
			else
			{
				// +-+
				// |/ \
				// +---+
				indexBuf[indexCount + 0] = vertexIndex(x, y);
				indexBuf[indexCount + 1] = vertexIndex(x, y + 1);
				indexBuf[indexCount + 2] = vertexIndex(x + 1, y);
				indexBuf[indexCount + 3] = vertexIndex(x + 1, y);
				indexBuf[indexCount + 4] = vertexIndex(x, y + 1);
				indexBuf[indexCount + 5] = vertexIndex(x + 2, y + 1);
				indexCount += 6;
			}
		}

		assert(indexCount <= maxIndexCount);
		uint32_t indexBytes = indexCount * sizeof(uint16_t);
		uint8_t meshTypeIndex = static_cast<uint8_t>(TerrainEdgeType::RightBottomSparse);
		vertexData.indexCounts[meshTypeIndex] = indexCount;
		renderDevice->SetBufferStorage(
			vertexData.indexBuffers[meshTypeIndex], indexBytes, indexBuf, BufferStorageFlags::None);
	}

	// MeshType_TopSparse
	{
		uint32_t indexCount = 0;

		for (int y = 0; y < sideQuads - 1; ++y)
			for (int x = 0; x < sideQuads; ++x)
				basicQuad(x, y, indexCount);

		for (int y = sideQuads - 1, x = 0; x < sideQuads; x += 2)
		{
			indexBuf[indexCount + 0] = vertexIndex(x, y);
			indexBuf[indexCount + 1] = vertexIndex(x, y + 1);
			indexBuf[indexCount + 2] = vertexIndex(x + 1, y);
			indexBuf[indexCount + 3] = vertexIndex(x + 1, y);
			indexBuf[indexCount + 4] = vertexIndex(x, y + 1);
			indexBuf[indexCount + 5] = vertexIndex(x + 2, y + 1);
			indexBuf[indexCount + 6] = vertexIndex(x + 1, y);
			indexBuf[indexCount + 7] = vertexIndex(x + 2, y + 1);
			indexBuf[indexCount + 8] = vertexIndex(x + 2, y);
			indexCount += 9;
		}

		assert(indexCount <= maxIndexCount);
		uint32_t indexBytes = indexCount * sizeof(uint16_t);
		uint8_t meshTypeIndex = static_cast<uint8_t>(TerrainEdgeType::BottomSparse);
		vertexData.indexCounts[meshTypeIndex] = indexCount;
		renderDevice->SetBufferStorage(
			vertexData.indexBuffers[meshTypeIndex], indexBytes, indexBuf, BufferStorageFlags::None);
	}

	// MeshType_BottomLeftSparse
	{
		uint32_t indexCount = 0;

		for (int y = 0; y < sideQuads - 1; ++y)
		{
			for (int x = 0; x < sideQuads; ++x)
			{
				if (x > 0)
					basicQuad(x, y, indexCount);
				else if ((y & 1) == 0)
				{
					// +-+
					// |\|
					// | +
					// |/
					// +
					indexBuf[indexCount + 0] = vertexIndex(x, y);
					indexBuf[indexCount + 1] = vertexIndex(x + 1, y + 1);
					indexBuf[indexCount + 2] = vertexIndex(x + 1, y);
					indexBuf[indexCount + 3] = vertexIndex(x, y);
					indexBuf[indexCount + 4] = vertexIndex(x, y + 2);
					indexBuf[indexCount + 5] = vertexIndex(x + 1, y + 1);
					indexCount += 6;

					if (y < sideQuads - 2)
					{
						//   +
						//  /|
						// +-+
						indexBuf[indexCount + 0] = vertexIndex(x + 1, y + 1);
						indexBuf[indexCount + 1] = vertexIndex(x, y + 2);
						indexBuf[indexCount + 2] = vertexIndex(x + 1, y + 2);
						indexCount += 3;
					}
				}
			}
		}

		for (int y = sideQuads - 1, x = 0; x < sideQuads; x += 2)
		{
			if (x > 0)
			{
				// +-+
				// |/
				// +
				indexBuf[indexCount + 0] = vertexIndex(x, y);
				indexBuf[indexCount + 1] = vertexIndex(x, y + 1);
				indexBuf[indexCount + 2] = vertexIndex(x + 1, y);
				indexCount += 3;
			}

			//   +-+
			//  / \|
			// +---+
			indexBuf[indexCount + 0] = vertexIndex(x, y + 1);
			indexBuf[indexCount + 1] = vertexIndex(x + 2, y + 1);
			indexBuf[indexCount + 2] = vertexIndex(x + 1, y);
			indexBuf[indexCount + 3] = vertexIndex(x + 1, y);
			indexBuf[indexCount + 4] = vertexIndex(x + 2, y + 1);
			indexBuf[indexCount + 5] = vertexIndex(x + 2, y);
			indexCount += 6;
		}

		assert(indexCount <= maxIndexCount);
		int indexBytes = indexCount * sizeof(uint16_t);
		uint8_t meshTypeIndex = static_cast<uint8_t>(TerrainEdgeType::BottomLeftSparse);
		vertexData.indexCounts[meshTypeIndex] = indexCount;
		renderDevice->SetBufferStorage(
			vertexData.indexBuffers[meshTypeIndex], indexBytes, indexBuf, BufferStorageFlags::None);
	}

	// MeshType_LeftSparse
	{
		uint32_t indexCount = 0;

		for (int y = 0; y < sideQuads; ++y)
		{
			for (int x = 0; x < sideQuads; ++x)
			{
				if (x > 0)
					basicQuad(x, y, indexCount);
				else if ((y & 1) == 0)
				{
					indexBuf[indexCount + 0] = vertexIndex(x, y);
					indexBuf[indexCount + 1] = vertexIndex(x + 1, y + 1);
					indexBuf[indexCount + 2] = vertexIndex(x + 1, y);
					indexBuf[indexCount + 3] = vertexIndex(x, y);
					indexBuf[indexCount + 4] = vertexIndex(x, y + 2);
					indexBuf[indexCount + 5] = vertexIndex(x + 1, y + 1);
					indexBuf[indexCount + 6] = vertexIndex(x + 1, y + 1);
					indexBuf[indexCount + 7] = vertexIndex(x, y + 2);
					indexBuf[indexCount + 8] = vertexIndex(x + 1, y + 2);
					indexCount += 9;
				}
			}
		}

		assert(indexCount <= maxIndexCount);
		uint32_t indexBytes = indexCount * sizeof(uint16_t);
		uint8_t meshTypeIndex = static_cast<uint8_t>(TerrainEdgeType::LeftSparse);
		vertexData.indexCounts[meshTypeIndex] = indexCount;
		renderDevice->SetBufferStorage(
			vertexData.indexBuffers[meshTypeIndex], indexBytes, indexBuf, BufferStorageFlags::None);
	}

	// MeshType_LeftTopSparse
	{
		uint32_t indexCount = 0;

		for (int y = 0, x = 0; x < sideQuads; x += 2)
		{
			if (x > 0)
			{
				// +
				// |\
				// +-+
				indexBuf[indexCount + 0] = vertexIndex(x, y);
				indexBuf[indexCount + 1] = vertexIndex(x, y + 1);
				indexBuf[indexCount + 2] = vertexIndex(x + 1, y + 1);
				indexCount += 3;
			}
			else
			{
				// +
				// |\
				// | +
				// |/|
				// +-+
				indexBuf[indexCount + 0] = vertexIndex(x, y);
				indexBuf[indexCount + 1] = vertexIndex(x, y + 2);
				indexBuf[indexCount + 2] = vertexIndex(x + 1, y + 1);
				indexBuf[indexCount + 3] = vertexIndex(x + 1, y + 1);
				indexBuf[indexCount + 4] = vertexIndex(x, y + 2);
				indexBuf[indexCount + 5] = vertexIndex(x + 1, y + 2);
				indexCount += 6;
			}

			// +---+
			//  \ /|
			//   +-+
			indexBuf[indexCount + 0] = vertexIndex(x, y);
			indexBuf[indexCount + 1] = vertexIndex(x + 1, y + 1);
			indexBuf[indexCount + 2] = vertexIndex(x + 2, y);
			indexBuf[indexCount + 3] = vertexIndex(x + 2, y);
			indexBuf[indexCount + 4] = vertexIndex(x + 1, y + 1);
			indexBuf[indexCount + 5] = vertexIndex(x + 2, y + 1);
			indexCount += 6;
		}

		for (int y = 1; y < sideQuads; ++y)
		{
			for (int x = 0; x < sideQuads; ++x)
			{
				if (x > 0)
					basicQuad(x, y, indexCount);
				else if ((y & 1) == 0)
				{
					indexBuf[indexCount + 0] = vertexIndex(x, y);
					indexBuf[indexCount + 1] = vertexIndex(x + 1, y + 1);
					indexBuf[indexCount + 2] = vertexIndex(x + 1, y);
					indexBuf[indexCount + 3] = vertexIndex(x, y);
					indexBuf[indexCount + 4] = vertexIndex(x, y + 2);
					indexBuf[indexCount + 5] = vertexIndex(x + 1, y + 1);
					indexBuf[indexCount + 6] = vertexIndex(x + 1, y + 1);
					indexBuf[indexCount + 7] = vertexIndex(x, y + 2);
					indexBuf[indexCount + 8] = vertexIndex(x + 1, y + 2);
					indexCount += 9;
				}
			}
		}

		assert(indexCount <= maxIndexCount);
		uint32_t indexBytes = indexCount * sizeof(uint16_t);
		uint8_t meshTypeIndex = static_cast<uint8_t>(TerrainEdgeType::LeftTopSparse);
		vertexData.indexCounts[meshTypeIndex] = indexCount;
		renderDevice->SetBufferStorage(
			vertexData.indexBuffers[meshTypeIndex], indexBytes, indexBuf, BufferStorageFlags::None);
	}

	VertexAttribute vertexAttributes[] = { VertexAttribute::pos2 };
	VertexFormat vertexFormatPos(vertexAttributes, KOKKO_ARRAY_ITEMS(vertexAttributes));
	vertexFormatPos.CalcOffsetsAndSizeInterleaved();
	const VertexAttribute& attr = vertexAttributes[0];

	// Initialize vertex buffers
	for (int i = 0; i < MeshTypeCount; ++i)
	{
		render::VertexArrayId vertexArray = vertexData.vertexArrays[i];
		renderDevice->SetVertexArrayVertexBuffer(vertexArray, 0, vertexData.vertexBuffer, 0, attr.stride);
		renderDevice->SetVertexArrayIndexBuffer(vertexArray, vertexData.indexBuffers[i]);

		renderDevice->EnableVertexAttribute(vertexArray, attr.attrIndex);
		renderDevice->SetVertexAttribFormat(vertexArray, attr.attrIndex, attr.elemCount, attr.elemType, attr.offset);
		renderDevice->SetVertexAttribBinding(vertexArray, attr.attrIndex, 0);
	}

	allocator->Deallocate(indexBuf);
	allocator->Deallocate(vertexBuf);
}

} // namespace kokko
