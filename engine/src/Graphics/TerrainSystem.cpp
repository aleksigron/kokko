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
#include "Resources/MeshManager.hpp"
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
	alignas(4) float terrainResolution;
	alignas(4) float heightOrigin;
	alignas(4) float heightRange;

	alignas(4) float metalness;
	alignas(4) float roughness;
};

TerrainSystem::TerrainSystem(
	Allocator* allocator,
	RenderDevice* renderDevice,
	ShaderManager* shaderManager,
	TextureManager* textureManager) :
	allocator(allocator),
	renderDevice(renderDevice),
	shaderManager(shaderManager),
	textureManager(textureManager),
	uniformBlockStride(0),
	uniformBufferId(0),
	terrainShader(ShaderId::Null),
	entityMap(allocator),
	vertexData(VertexData{}),
	textureSampler(0),
	tilesToRender(allocator),
	uniformStagingBuffer(allocator)
{
	data = InstanceData{};
	data.count = 1; // Reserve index 0 as TerrainId::Null value

	this->Reallocate(16);
}

TerrainSystem::~TerrainSystem()
{
	RemoveAll();

	if (vertexData.indexBuffer != 0)
		renderDevice->DestroyBuffers(1, &vertexData.indexBuffer);

	if (vertexData.vertexBuffer != 0)
		renderDevice->DestroyBuffers(1, &vertexData.vertexBuffer);

	if (vertexData.vertexArray != 0)
		renderDevice->DestroyVertexArrays(1, &vertexData.vertexArray);

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

	uniformBlockStride = (sizeof(TerrainUniformBlock) + aligment - 1) / aligment * aligment;

	// TODO: recreate buffer based on tree levels and tile count
	int tileCount = TerrainQuadTree::GetTileCountForLevelCount(6);
	int uniformBytes = uniformBlockStride * tileCount;

	renderDevice->CreateBuffers(1, &uniformBufferId);
	renderDevice->SetBufferStorage(uniformBufferId, uniformBytes, nullptr, BufferStorageFlags::Dynamic);

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

	CreateVertexData();
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
	data.quadTree[id] = TerrainQuadTree();

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

void TerrainSystem::SetAlbedoTexture(TerrainId id, TextureId textureId, RenderTextureId textureObject)
{
	data.textures[id.i].albedoTexture.textureId = textureId;
	data.textures[id.i].albedoTexture.textureObjectId = textureObject;
}

TextureId TerrainSystem::GetRoughnessTextureId(TerrainId id) const
{
	return data.textures[id.i].roughnessTexture.textureId;
}

void TerrainSystem::SetRoughnessTexture(TerrainId id, TextureId textureId, RenderTextureId textureObject)
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

void TerrainSystem::CreateVertexData()
{
	// Create vertex data

	int sideVerts = TerrainTile::Resolution + 1;
	unsigned int vertCount = sideVerts * sideVerts;
	size_t vertexComponents = 2;
	size_t vertSize = sizeof(float) * vertexComponents;
	unsigned int vertBytes = static_cast<unsigned int>(vertSize * vertCount);
	float* vertexBuf = static_cast<float*>(allocator->Allocate(vertBytes, "TerrainSystem.CreateVertexData() vertexBuf"));

	int sideQuads = TerrainTile::Resolution;
	int quadIndices = 3 * 2; // 3 indices per triangle, 2 triangles per quad
	unsigned int indexCount = sideQuads * sideQuads * quadIndices;
	unsigned int indexBytes = indexCount * sizeof(uint16_t);
	uint16_t* indexBuf = static_cast<uint16_t*>(allocator->Allocate(indexBytes, "TerrainSystem.CreateVertexData() indexBuf"));

	float quadSize = 1.0f / TerrainTile::Resolution;

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

	// Set index data
	for (size_t y = 0; y < sideQuads; ++y)
	{
		for (size_t x = 0; x < sideQuads; ++x)
		{
			size_t quadStart = (y * sideQuads + x) * quadIndices;
			indexBuf[quadStart + 0] = static_cast<uint16_t>(y * sideVerts + x);
			indexBuf[quadStart + 1] = static_cast<uint16_t>((y + 1) * sideVerts + x);
			indexBuf[quadStart + 2] = static_cast<uint16_t>(y * sideVerts + (x + 1));
			indexBuf[quadStart + 3] = static_cast<uint16_t>((y + 1) * sideVerts + x);
			indexBuf[quadStart + 4] = static_cast<uint16_t>((y + 1) * sideVerts + (x + 1));
			indexBuf[quadStart + 5] = static_cast<uint16_t>(y * sideVerts + (x + 1));
		}
	}

	renderDevice->CreateVertexArrays(1, &vertexData.vertexArray);
	renderDevice->CreateBuffers(1, &vertexData.vertexBuffer);
	renderDevice->CreateBuffers(1, &vertexData.indexBuffer);
	vertexData.indexCount = indexCount;

	// Bind and upload index buffer
	renderDevice->SetBufferStorage(vertexData.indexBuffer, indexBytes, indexBuf, BufferStorageFlags::None);

	// Bind and upload vertex buffer
	renderDevice->SetBufferStorage(vertexData.vertexBuffer, vertBytes, vertexBuf, BufferStorageFlags::None);

	VertexAttribute vertexAttributes[] = { VertexAttribute::pos2 };
	VertexFormat vertexFormatPos(vertexAttributes, KOKKO_ARRAY_ITEMS(vertexAttributes));
	vertexFormatPos.CalcOffsetsAndSizeInterleaved();
	const VertexAttribute& attr = vertexAttributes[0];

	renderDevice->SetVertexArrayVertexBuffer(vertexData.vertexArray, 0, vertexData.vertexBuffer, 0, attr.stride);
	renderDevice->SetVertexArrayIndexBuffer(vertexData.vertexArray, vertexData.indexBuffer);

	renderDevice->EnableVertexAttribute(vertexData.vertexArray, attr.attrIndex);
	renderDevice->SetVertexAttribFormat(
		vertexData.vertexArray, attr.attrIndex, attr.elemCount, attr.elemType, attr.offset);
	renderDevice->SetVertexAttribBinding(vertexData.vertexArray, attr.attrIndex, 0);

	allocator->Deallocate(indexBuf);
	allocator->Deallocate(vertexBuf);
}

void TerrainSystem::InitializeTerrain(TerrainId id, const TerrainParameters& params)
{
	KOKKO_PROFILE_FUNCTION();

	auto scope = renderDevice->CreateDebugScope(0, ConstStringView("TerrainSys_CreateInstanceResources"));

	kokko::TerrainQuadTree& quadTree = data.quadTree[id.i];

	constexpr int treeLevels = 7;
	quadTree.CreateResources(allocator, renderDevice, treeLevels, params);
}

void TerrainSystem::DeinitializeTerrain(TerrainId id)
{
	KOKKO_PROFILE_FUNCTION();

	auto scope = renderDevice->CreateDebugScope(0, ConstStringView("TerrainSys_DestroyInstanceResources"));

	data.quadTree[id.i].DestroyResources(allocator, renderDevice);
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
	quadTree.GetTilesToRender(viewport.frustum, viewport.viewProjection, tilesToRender);

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
	uniforms.terrainResolution = static_cast<float>(TerrainTile::Resolution);
	uniforms.heightOrigin = quadTree.GetBottom();
	uniforms.heightRange = terrainHeight;
	uniforms.metalness = 0.0f;
	uniforms.roughness = 0.5f;

	uniformStagingBuffer.Resize(tilesToRender.GetCount() * uniformBlockStride);

	int blocksWritten = 0;
	for (const auto& tile : tilesToRender)
	{
		const float halfTileCount = 0.5f * TerrainQuadTree::GetTilesPerDimension(tile.level);
		const Vec2f levelOrigin(-halfTileCount, -halfTileCount);

		uniforms.tileOffset = levelOrigin + Vec2f(static_cast<float>(tile.x), static_cast<float>(tile.y));
		uniforms.tileScale = TerrainQuadTree::GetTileScale(tile.level);

		uint8_t* dest = &uniformStagingBuffer[blocksWritten * uniformBlockStride];
		std::memcpy(dest, &uniforms, sizeof(uniforms));

		blocksWritten += 1;
	}

	unsigned int updateBytes = static_cast<unsigned int>(tilesToRender.GetCount() * uniformBlockStride);

	renderDevice->SetBufferSubData(uniformBufferId, 0, updateBytes, uniformStagingBuffer.GetData());

	const ShaderData& shader = shaderManager->GetShaderData(terrainShader);

	// Draw
	encoder->UseShaderProgram(shader.driverId);

	const TextureUniform* heightMap = shader.uniforms.FindTextureUniformByNameHash("height_map"_hash);
	const TextureUniform* albedoMap = shader.uniforms.FindTextureUniformByNameHash("albedo_map"_hash);

	if (albedoMap != nullptr)
	{
		kokko::RenderTextureId textureObjectId;
		if (data.textures[id.i].albedoTexture.textureObjectId != 0)
			textureObjectId = data.textures[id.i].albedoTexture.textureObjectId;
		else
		{
			TextureId emptyAlbedoId = textureManager->GetId_White2D();
			textureObjectId = textureManager->GetTextureData(emptyAlbedoId).textureObjectId;
		}

		encoder->BindTextureToShader(albedoMap->uniformLocation, 1, textureObjectId);
	}

	encoder->BindVertexArray(vertexData.vertexArray);

	// For height texture

	encoder->BindSampler(0, textureSampler);

	int blocksUsed = 0;
	for (const auto& tile : tilesToRender)
	{
		intptr_t rangeOffset = blocksUsed * uniformBlockStride;
		blocksUsed += 1;

		encoder->BindBufferRange(RenderBufferTarget::UniformBuffer, UniformBlockBinding::Object, uniformBufferId,
			rangeOffset, uniformBlockStride);

		if (heightMap != nullptr)
		{
			RenderTextureId heightTex = quadTree.GetTileHeightTexture(tile.level, tile.x, tile.y);

			encoder->BindTextureToShader(heightMap->uniformLocation, 0, heightTex);
		}

		encoder->DrawIndexed(
			RenderPrimitiveMode::Triangles, RenderIndexType::UnsignedShort, vertexData.indexCount, 0, 0);
	}

	tilesToRender.Clear();
}

}
