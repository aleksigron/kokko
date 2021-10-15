#include "Graphics/TerrainSystem.hpp"

#include <cassert>

#include "Core/Core.hpp"

#include "Graphics/TerrainQuadTree.hpp"

#include "Math/Mat4x4.hpp"
#include "Math/Vec3.hpp"
#include "Math/Vec2.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/Renderer.hpp"
#include "Rendering/RenderCommandList.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/RenderViewport.hpp"
#include "Rendering/StaticUniformBuffer.hpp"
#include "Rendering/VertexFormat.hpp"

#include "Resources/MaterialManager.hpp"
#include "Resources/MeshManager.hpp"

namespace kokko
{

const TerrainId TerrainId::Null = TerrainId{ 0 };

struct TerrainUniformBlock
{
	static const unsigned int BindingPoint = 2;

	alignas(16) Mat4x4f MVP;
	alignas(16) Mat4x4f MV;

	alignas(8) Vec2f textureScale;

	alignas(4) float terrainSize;
	alignas(4) float terrainResolution;
	alignas(4) float minHeight;
	alignas(4) float maxHeight;
};

TerrainSystem::TerrainSystem(
	Allocator* allocator,
	RenderDevice* renderDevice,
	MaterialManager* materialManager,
	ShaderManager* shaderManager) :
	allocator(allocator),
	renderDevice(renderDevice),
	materialManager(materialManager),
	shaderManager(shaderManager),
	terrainMaterial(MaterialId::Null),
	entityMap(allocator),
	vertexData(VertexData{})
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
}

void TerrainSystem::Initialize()
{
	KOKKO_PROFILE_FUNCTION();

	StringRef path("engine/materials/deferred_geometry/terrain.material.json");
	terrainMaterial = materialManager->GetIdByPath(path);

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
	data.param[id] = params;
	data.resource[id] = ResourceData{};

	data.count += 1;

	TerrainId terrainId{ id };
	InitializeTerrain(terrainId);

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
		data.param[id.i] = data.param[swapIdx];
		data.resource[id.i] = data.resource[swapIdx];
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

void TerrainSystem::RegisterCustomRenderer(Renderer* renderer)
{
	renderer->AddCustomRenderer(this);
}

void TerrainSystem::AddRenderCommands(const CustomRenderer::CommandParams& params)
{
	params.commandList->AddDrawWithCallback(params.fullscreenViewport, RenderPass::OpaqueGeometry, 0.0f, params.callbackId);
}

void TerrainSystem::RenderCustom(const CustomRenderer::RenderParams& params)
{
	const MaterialData& material = materialManager->GetMaterialData(terrainMaterial);

	for (size_t i = 1; i < data.count; ++i)
	{
		RenderTerrain(TerrainId{ static_cast<unsigned int>(i) }, material, *params.viewport);
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
	float* vertexBuf = static_cast<float*>(allocator->Allocate(vertBytes));

	int sideQuads = TerrainTile::Resolution;
	int quadIndices = 3 * 2; // 3 indices per triangle, 2 triangles per quad
	unsigned int indexCount = sideQuads * sideQuads * quadIndices;
	unsigned int indexBytes = indexCount * sizeof(uint16_t);
	uint16_t* indexBuf = static_cast<uint16_t*>(allocator->Allocate(indexBytes));

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

	renderDevice->BindVertexArray(vertexData.vertexArray);

	RenderBufferUsage usage = RenderBufferUsage::StaticDraw;

	// Bind and upload index buffer
	renderDevice->BindBuffer(RenderBufferTarget::IndexBuffer, vertexData.indexBuffer);
	renderDevice->SetBufferData(RenderBufferTarget::IndexBuffer, indexBytes, indexBuf, usage);

	// Bind and upload vertex buffer
	renderDevice->BindBuffer(RenderBufferTarget::VertexBuffer, vertexData.vertexBuffer);
	renderDevice->SetBufferData(RenderBufferTarget::VertexBuffer, vertBytes, vertexBuf, usage);

	VertexAttribute vertexAttributes[] = { VertexAttribute::pos2 };
	VertexFormat vertexFormatPos(vertexAttributes, sizeof(vertexAttributes) / sizeof(vertexAttributes[0]));

	const VertexAttribute& attr = vertexAttributes[0];
	renderDevice->EnableVertexAttribute(attr.attrIndex);

	int stride = static_cast<int>(vertexFormatPos.vertexSize);

	RenderCommandData::SetVertexAttributePointer data{
		attr.attrIndex, attr.elemCount, attr.elemType, stride, attr.offset
	};

	renderDevice->SetVertexAttributePointer(&data);

	allocator->Deallocate(indexBuf);
	allocator->Deallocate(vertexBuf);
}

void TerrainSystem::InitializeTerrain(TerrainId id)
{
	KOKKO_PROFILE_FUNCTION();

	const TerrainParameters& params = data.param[id.i];
	ResourceData& res = data.resource[id.i];

	// Create uniform buffer

	renderDevice->CreateBuffers(1, &res.uniformBufferId);
	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, res.uniformBufferId);

	RenderCommandData::SetBufferStorage bufferStorage{};
	bufferStorage.target = RenderBufferTarget::UniformBuffer;
	bufferStorage.size = sizeof(TerrainUniformBlock);
	bufferStorage.data = nullptr;
	bufferStorage.dynamicStorage = true;
	renderDevice->SetBufferStorage(&bufferStorage);

	res.quadTree.CreateResources(allocator, renderDevice, 4);
}

void TerrainSystem::DeinitializeTerrain(TerrainId id)
{
	KOKKO_PROFILE_FUNCTION();

	data.resource[id.i].quadTree.DestroyResources(allocator, renderDevice);
}

void TerrainSystem::Reallocate(size_t required)
{
	if (required <= data.allocated)
		return;

	required = static_cast<unsigned int>(Math::UpperPowerOfTwo(required));

	// Reserve same amount in entity map
	entityMap.Reserve(required);

	InstanceData newData;
	size_t bytes = required * (sizeof(Entity) + sizeof(TerrainParameters) + sizeof(ResourceData));

	newData.buffer = allocator->Allocate(bytes);
	newData.count = data.count;
	newData.allocated = required;

	newData.entity = static_cast<Entity*>(newData.buffer);
	newData.param = reinterpret_cast<TerrainParameters*>(newData.entity + required);
	newData.resource = reinterpret_cast<ResourceData*>(newData.param + required);

	if (data.buffer != nullptr)
	{
		std::memcpy(newData.entity, data.entity, data.count * sizeof(Entity));
		std::memcpy(newData.param, data.param, data.count * sizeof(TerrainParameters));
		std::memcpy(newData.resource, data.resource, data.count * sizeof(ResourceData));

		allocator->Deallocate(data.buffer);
	}

	data = newData;
}

void TerrainSystem::RenderTerrain(TerrainId id, const MaterialData& material, const RenderViewport& viewport)
{
	KOKKO_PROFILE_FUNCTION();

	const TerrainParameters& params = data.param[id.i];

	TerrainUniformBlock uniforms;
	uniforms.MVP = viewport.viewProjection;
	uniforms.MV = viewport.view;
	uniforms.textureScale = params.textureScale;
	uniforms.terrainSize = params.terrainSize;
	uniforms.terrainResolution = static_cast<float>(TerrainTile::Resolution);
	uniforms.minHeight = params.minHeight;
	uniforms.maxHeight = params.maxHeight;

	const ResourceData& resources = data.resource[id.i];

	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, resources.uniformBufferId);
	renderDevice->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, sizeof(TerrainUniformBlock), &uniforms);

	// Bind object transform uniform block to shader
	renderDevice->BindBufferBase(RenderBufferTarget::UniformBuffer, UniformBlockBinding::Object, resources.uniformBufferId);

	renderDevice->UseShaderProgram(material.cachedShaderDeviceId);

	const TextureUniform* heightMap = material.uniforms.FindTextureUniformByNameHash("height_map"_hash);
	const TextureUniform* albedoMap = material.uniforms.FindTextureUniformByNameHash("albedo_map"_hash);
	const TextureUniform* roughMap = material.uniforms.FindTextureUniformByNameHash("roughness_map"_hash);

	TerrainQuadTree& quadTree = data.resource[id.i].quadTree;

	int levels = quadTree.GetLevelCount();
	int currentLevel = levels - 1;
	int tilesPerDimension = TerrainQuadTree::GetTilesPerDimension(currentLevel);
	int tileCount = tilesPerDimension * tilesPerDimension;

	renderDevice->BindVertexArray(vertexData.vertexArray);

	for (int x = 0; x < tilesPerDimension; ++x)
	{
		for (int y = 0; y < tilesPerDimension; ++y)
		{
			auto tile = quadTree.GetTile(currentLevel, x, y);

			if (heightMap != nullptr)
			{
				unsigned int heigthTex = quadTree.GetTileHeightTexture(currentLevel, x, y);

				renderDevice->SetUniformInt(heightMap->uniformLocation, 0);
				renderDevice->SetActiveTextureUnit(0);
				renderDevice->BindTexture(RenderTextureTarget::Texture2d, heigthTex);
			}

			if (albedoMap != nullptr)
			{
				renderDevice->SetUniformInt(albedoMap->uniformLocation, 1);
				renderDevice->SetActiveTextureUnit(1);
				renderDevice->BindTexture(RenderTextureTarget::Texture2d, albedoMap->textureObject);
			}

			if (roughMap != nullptr)
			{
				renderDevice->SetUniformInt(roughMap->uniformLocation, 2);
				renderDevice->SetActiveTextureUnit(2);
				renderDevice->BindTexture(RenderTextureTarget::Texture2d, roughMap->textureObject);
			}

			renderDevice->BindBufferBase(RenderBufferTarget::UniformBuffer, UniformBlockBinding::Material, material.uniformBufferObject);

			renderDevice->DrawIndexed(RenderPrimitiveMode::Triangles, vertexData.indexCount,
				RenderIndexType::UnsignedShort);
		}
	}
}

}
