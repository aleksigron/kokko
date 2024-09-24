#include "Graphics/TerrainSystem.hpp"

#include <cassert>
#include <cmath>
#include <cstring>

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

TerrainSystem::TerrainSystem(
	Allocator* allocator,
	kokko::render::Device* renderDevice,
	ShaderManager* shaderManager,
	TextureManager* textureManager) :
	allocator(allocator),
	renderDevice(renderDevice),
	shaderManager(shaderManager),
	textureManager(textureManager),
	uniformBlockStride(0),
	uniformBlocksAllocated(0),
	uniformBufferId(0),
	terrainShader(ShaderId::Null),
	entityMap(allocator),
	vertexData(VertexData{}),
	textureSampler(0),
	uniformStagingBuffer(allocator)
{
	data = InstanceData{};
	data.count = 1; // Reserve index 0 as TerrainId::Null value

	this->Reallocate(16);
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

	allocator->Deallocate(data.buffer);
}

void TerrainSystem::Initialize()
{
	KOKKO_PROFILE_FUNCTION();

	auto scope = renderDevice->CreateDebugScope(0, ConstStringView("TerrainSys_InitResources"));

	// Create uniform buffer

	int aligment = 0;
	renderDevice->GetIntegerValue(RenderDeviceParameter::UniformBufferOffsetAlignment, &aligment);

	uniformBlockStride = Math::RoundUpToMultiple(sizeof(TerrainUniformBlock), static_cast<size_t>(aligment));

	// Material

	ConstStringView path("engine/shaders/deferred_geometry/terrain.glsl");
	terrainShader = shaderManager->FindShaderByPath(path);
	assert(terrainShader != ShaderId::Null);

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

TerrainId TerrainSystem::AddTerrain(Entity entity, const TerrainParameters& params)
{
	if (data.count + 1 > data.allocated)
		this->Reallocate(data.count + 1);

	unsigned int id = static_cast<unsigned int>(data.count);

	auto mapPair = entityMap.Insert(entity.id);
	mapPair->second.i = id;

	data.entity[id] = entity;
	data.textureScale[id] = params.textureScale;
	data.textures[id] = TerrainTextures{};
	new (&data.quadTree[id]) TerrainQuadTree(allocator, renderDevice);

	data.count += 1;

	TerrainId terrainId{ id };
	InitializeTerrain(terrainId, params);

	return terrainId;
}

void TerrainSystem::RemoveTerrain(TerrainId id)
{
	assert(id != TerrainId::Null);
	assert(id.i < data.count);

	DeinitializeTerrain(id);

	// Remove from entity map
	Entity entity = data.entity[id.i];
	auto* pair = entityMap.Lookup(entity.id);
	if (pair != nullptr)
		entityMap.Remove(pair);

	if (data.count > 2 && id.i + 1 < data.count) // We need to swap another object
	{
		size_t swapIdx = data.count - 1;

		// Update the swapped objects id in the entity map
		auto* swapKv = entityMap.Lookup(data.entity[swapIdx].id);
		if (swapKv != nullptr)
			swapKv->second = id;

		data.entity[id.i] = data.entity[swapIdx];
		data.textureScale[id.i] = data.textureScale[swapIdx];
		data.quadTree[id.i] = data.quadTree[swapIdx];
	}

	--data.count;
}

void TerrainSystem::RemoveAll()
{
	for (size_t i = 1; i < data.count; ++i)
		DeinitializeTerrain(TerrainId{ static_cast<unsigned int>(i) });

	entityMap.Clear();
	data.count = 1;
}

TextureId TerrainSystem::GetAlbedoTextureId(TerrainId id) const
{
	return data.textures[id.i].albedoTexture.textureId;
}

void TerrainSystem::SetAlbedoTexture(TerrainId id, TextureId textureId, render::TextureId textureObject)
{
	data.textures[id.i].albedoTexture.textureId = textureId;
	data.textures[id.i].albedoTexture.textureObjectId = textureObject;
}

TextureId TerrainSystem::GetRoughnessTextureId(TerrainId id) const
{
	return data.textures[id.i].roughnessTexture.textureId;
}

void TerrainSystem::SetRoughnessTexture(TerrainId id, TextureId textureId, render::TextureId textureObject)
{
	data.textures[id.i].roughnessTexture.textureId = textureId;
	data.textures[id.i].roughnessTexture.textureObjectId = textureObject;
}

void TerrainSystem::Submit(const SubmitParameters& parameters)
{
	for (size_t i = 1; i < data.count; ++i)
	{
		float depth = 0.0f; // TODO: Calculate
		parameters.commandList.AddToFullscreenViewport(RenderPassType::OpaqueGeometry, depth, i);
	}
}

void TerrainSystem::InitializeTerrain(TerrainId id, const TerrainParameters& params)
{
	KOKKO_PROFILE_FUNCTION();

	auto scope = renderDevice->CreateDebugScope(0, ConstStringView("TerrainSys_CreateInstanceResources"));

	kokko::TerrainQuadTree& quadTree = data.quadTree[id.i];

	constexpr int treeLevels = 7;
	quadTree.Initialize(treeLevels, params);
}

void TerrainSystem::DeinitializeTerrain(TerrainId id)
{
	KOKKO_PROFILE_FUNCTION();

	auto scope = renderDevice->CreateDebugScope(0, ConstStringView("TerrainSys_DestroyInstanceResources"));

	data.quadTree[id.i].~TerrainQuadTree();
}

void TerrainSystem::Reallocate(size_t required)
{
	if (required <= data.allocated)
		return;

	required = static_cast<unsigned int>(Math::UpperPowerOfTwo(required));

	// Reserve same amount in entity map
	entityMap.Reserve(required);

	InstanceData newData;
	size_t bytes = required * (sizeof(Entity) + sizeof(Vec2f) + sizeof(TerrainQuadTree));

	newData.buffer = allocator->Allocate(bytes, "TerrainSystem.data.buffer");
	newData.count = data.count;
	newData.allocated = required;

	newData.entity = static_cast<Entity*>(newData.buffer);
	newData.textureScale = reinterpret_cast<Vec2f*>(newData.entity + required);
	newData.textures = reinterpret_cast<TerrainTextures*>(newData.textureScale + required);
	newData.quadTree = reinterpret_cast<TerrainQuadTree*>(newData.textures + required);

	if (data.buffer != nullptr)
	{
		std::memcpy(newData.entity, data.entity, data.count * sizeof(Entity));
		std::memcpy(newData.textureScale, data.textureScale, data.count * sizeof(Vec2f));
		std::memcpy(newData.textures, data.textures, data.count * sizeof(TerrainTextures));
		std::memcpy(newData.quadTree, data.quadTree, data.count * sizeof(TerrainQuadTree));

		allocator->Deallocate(data.buffer);
	}

	data = newData;
}

void TerrainSystem::Render(const RenderParameters& parameters)
{
	KOKKO_PROFILE_FUNCTION();

	TerrainId id{ static_cast<uint32_t>(parameters.featureObjectId) };
	const auto& viewport = parameters.fullscreenViewport;
	render::CommandEncoder* encoder = parameters.encoder;

	// Select tiles to render

	TerrainQuadTree& quadTree = data.quadTree[id.i];
	quadTree.UpdateTilesToRender(viewport.frustum, viewport.position, parameters.renderDebug);
	ArrayView<const TerrainTileDrawInfo> tiles = quadTree.GetTilesToRender();
	uint32_t tileCount = static_cast<uint32_t>(tiles.GetCount());

	// Update uniform buffer

	float terrainWidth = quadTree.GetSize();
	float terrainHeight = quadTree.GetHeight();

	TerrainUniformBlock uniforms;
	uniforms.MVP = viewport.viewProjection;
	uniforms.MV = viewport.view.inverse;
	uniforms.textureScale = data.textureScale[id.i];
	uniforms.tileOffset = Vec2f();
	uniforms.tileScale = 1.0f;
	uniforms.terrainSize = terrainWidth;
	uniforms.terrainSideVerts = static_cast<float>(TerrainTile::VerticesPerSide);
	uniforms.heightOrigin = quadTree.GetBottom();
	uniforms.heightRange = terrainHeight;
	uniforms.metalness = 0.0f;
	uniforms.roughness = 0.5f;

	uniformStagingBuffer.Resize(tiles.GetCount() * uniformBlockStride);

	if (tileCount > uniformBlocksAllocated)
	{
		uint32_t newAllocated = Math::RoundUpToMultiple(tileCount * 4 / 3, 16u);
		uint32_t newByteCount = newAllocated * uniformBlockStride;

		if (uniformBufferId != render::BufferId::Null)
			renderDevice->DestroyBuffers(1, &uniformBufferId);

		renderDevice->CreateBuffers(1, &uniformBufferId);
		renderDevice->SetBufferStorage(uniformBufferId, newByteCount, nullptr, BufferStorageFlags::Dynamic);

		uniformBlocksAllocated = newAllocated;
	}

	{
		KOKKO_PROFILE_SCOPE("Setup tile uniform data");

		uint32_t blocksWritten = 0;
		for (const auto& tile : tiles)
		{
			const float halfTileCount = 0.5f * TerrainQuadTree::GetTilesPerDimension(tile.id.level);
			const Vec2f levelOrigin(-halfTileCount, -halfTileCount);

			uniforms.tileOffset = levelOrigin + Vec2f(static_cast<float>(tile.id.x), static_cast<float>(tile.id.y));
			uniforms.tileScale = TerrainQuadTree::GetTileScale(tile.id.level);

			uint8_t* dest = &uniformStagingBuffer[blocksWritten * uniformBlockStride];
			std::memcpy(dest, &uniforms, sizeof(uniforms));

			blocksWritten += 1;
		}
	}

	uint32_t updateBytes = tileCount * uniformBlockStride;

	renderDevice->SetBufferSubData(uniformBufferId, 0, updateBytes, uniformStagingBuffer.GetData());

	const ShaderData& shader = shaderManager->GetShaderData(terrainShader);

	// Draw
	encoder->UseShaderProgram(shader.driverId);

	const TextureUniform* heightMap = shader.uniforms.FindTextureUniformByNameHash("height_map"_hash);
	const TextureUniform* albedoMap = shader.uniforms.FindTextureUniformByNameHash("albedo_map"_hash);

	if (albedoMap != nullptr)
	{
		kokko::render::TextureId textureObjectId;
		if (data.textures[id.i].albedoTexture.textureObjectId != 0)
			textureObjectId = data.textures[id.i].albedoTexture.textureObjectId;
		else
		{
			TextureId emptyAlbedoId = textureManager->GetId_White2D();
			textureObjectId = textureManager->GetTextureData(emptyAlbedoId).textureObjectId;
		}

		encoder->BindTextureToShader(albedoMap->uniformLocation, 1, textureObjectId);
	}

	// For height texture

	encoder->BindSampler(0, textureSampler);

	{
		KOKKO_PROFILE_SCOPE("Render tiles");

		uint8_t prevMeshType = 255;
		int blocksUsed = 0;
		for (const auto& tile : tiles)
		{
			uint8_t tileMeshTypeIndex = static_cast<uint8_t>(tile.edgeType);
			if (tileMeshTypeIndex != prevMeshType)
			{
				encoder->BindVertexArray(vertexData.vertexArrays[tileMeshTypeIndex]);
				prevMeshType = tileMeshTypeIndex;
			}

			intptr_t rangeOffset = blocksUsed * uniformBlockStride;
			blocksUsed += 1;

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
