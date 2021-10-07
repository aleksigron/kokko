#include "Graphics/TerrainSystem.hpp"

#include <cassert>

#include "Core/Core.hpp"

#include "Graphics/TerrainInstance.hpp"

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
	MeshManager* meshManager,
	MaterialManager* materialManager,
	ShaderManager* shaderManager) :
	allocator(allocator),
	renderDevice(renderDevice),
	meshManager(meshManager),
	materialManager(materialManager),
	shaderManager(shaderManager),
	terrainMaterial(MaterialId::Null),
	entityMap(allocator)
{
	data = InstanceData{};
	data.count = 1; // Reserve index 0 as TerrainId::Null value

	this->Reallocate(16);
}

TerrainSystem::~TerrainSystem()
{
	RemoveAll();
}

void TerrainSystem::Initialize()
{
	KOKKO_PROFILE_FUNCTION();

	StringRef path("engine/materials/deferred_geometry/terrain.material.json");
	terrainMaterial = materialManager->GetIdByPath(path);
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

	unsigned int id = data.count;

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
		unsigned int swapIdx = data.count - 1;

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

void TerrainSystem::InitializeTerrain(TerrainId id)
{
	KOKKO_PROFILE_FUNCTION();

	const TerrainParameters& params = data.param[id.i];
	ResourceData& res = data.resource[id.i];

	// Create texture data

	int texSize = static_cast<int>(params.terrainResolution);
	float texSizeInv = 1.0f / texSize;
	float scale = 64.0f;
	size_t dataSizeBytes = texSize * texSize * sizeof(uint16_t);
	res.heightData = static_cast<uint16_t*>(allocator->Allocate(dataSizeBytes));

	for (size_t y = 0; y < texSize; ++y)
	{
		for (size_t x = 0; x < texSize; ++x)
		{
			size_t pixelIndex = y * texSize + x;
			float normalized = 0.5f + std::sin(x * texSizeInv * scale) * 0.25f + std::sin(y * texSizeInv * scale) * 0.25f;
			uint16_t height = static_cast<uint16_t>(normalized * UINT16_MAX);
			res.heightData[pixelIndex] = height;
		}
	}

	renderDevice->CreateTextures(1, &res.textureId);
	renderDevice->BindTexture(RenderTextureTarget::Texture2d, res.textureId);

	RenderCommandData::SetTextureStorage2D textureStorage{
		RenderTextureTarget::Texture2d, 1, RenderTextureSizedFormat::R16, texSize, texSize
	};
	renderDevice->SetTextureStorage2D(&textureStorage);

	RenderCommandData::SetTextureSubImage2D subimage{
		RenderTextureTarget::Texture2d, 0, 0, 0, texSize, texSize, RenderTextureBaseFormat::R,
		RenderTextureDataType::UnsignedShort, res.heightData
	};
	renderDevice->SetTextureSubImage2D(&subimage);

	// Create vertex data

	int sideVerts = params.terrainResolution + 1;
	unsigned int vertCount = sideVerts * sideVerts;
	size_t vertexComponents = 2;
	size_t vertSize = sizeof(float) * vertexComponents;
	float* vertexData = static_cast<float*>(allocator->Allocate(vertCount * vertSize));

	int sideQuads = params.terrainResolution;
	int quadIndices = 3 * 2; // 3 indices per triangle, 2 triangles per quad
	unsigned int indexCount = sideQuads * sideQuads * quadIndices;
	uint16_t* indexData = static_cast<uint16_t*>(allocator->Allocate(indexCount * sizeof(uint16_t)));
	// TODO: Use 32-bit index type if necessary

	float quadSize = 1.0f / params.terrainSize;

	// Set vertex data
	for (size_t y = 0; y < sideVerts; ++y)
	{
		for (size_t x = 0; x < sideVerts; ++x)
		{
			size_t vertIndex = y * sideVerts + x;
			vertexData[vertIndex * vertexComponents + 0] = x * quadSize;
			vertexData[vertIndex * vertexComponents + 1] = y * quadSize;
		}
	}

	// Set index data
	for (size_t y = 0; y < sideQuads; ++y)
	{
		for (size_t x = 0; x < sideQuads; ++x)
		{
			size_t quadStart = (y * sideQuads + x) * quadIndices;
			indexData[quadStart + 0] = static_cast<uint16_t>(y * sideVerts + x);
			indexData[quadStart + 1] = static_cast<uint16_t>((y + 1) * sideVerts + x);
			indexData[quadStart + 2] = static_cast<uint16_t>(y * sideVerts + (x + 1));
			indexData[quadStart + 3] = static_cast<uint16_t>((y + 1) * sideVerts + x);
			indexData[quadStart + 4] = static_cast<uint16_t>((y + 1) * sideVerts + (x + 1));
			indexData[quadStart + 5] = static_cast<uint16_t>(y * sideVerts + (x + 1));
		}
	}

	VertexAttribute vertexAttributes[] = { VertexAttribute::pos2 };
	VertexFormat vertexFormatPos(vertexAttributes, sizeof(vertexAttributes) / sizeof(vertexAttributes[0]));

	IndexedVertexData vertData;
	vertData.vertexFormat = vertexFormatPos;
	vertData.primitiveMode = RenderPrimitiveMode::Triangles;
	vertData.vertexData = vertexData;
	vertData.vertexCount = vertCount;
	vertData.indexData = indexData;
	vertData.indexCount = indexCount;

	res.meshId = meshManager->CreateMesh();
	meshManager->UploadIndexed(res.meshId, vertData);

	float halfTerrainSize = params.terrainSize * 0.5f;

	BoundingBox bounds;
	bounds.center = Vec3f(0.0f, 0.0f, 0.0f);
	bounds.extents = Vec3f(halfTerrainSize, halfTerrainSize, halfTerrainSize);
	meshManager->SetBoundingBox(res.meshId, bounds);

	allocator->Deallocate(indexData);
	allocator->Deallocate(vertexData);

	// Create uniform buffer

	renderDevice->CreateBuffers(1, &res.uniformBufferId);
	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, res.uniformBufferId);

	RenderCommandData::SetBufferStorage bufferStorage{};
	bufferStorage.target = RenderBufferTarget::UniformBuffer;
	bufferStorage.size = sizeof(TerrainUniformBlock);
	bufferStorage.data = nullptr;
	bufferStorage.dynamicStorage = true;
	renderDevice->SetBufferStorage(&bufferStorage);
}

void TerrainSystem::DeinitializeTerrain(TerrainId id)
{
	KOKKO_PROFILE_FUNCTION();

	ResourceData& resources = data.resource[id.i];

	allocator->Deallocate(resources.heightData);

	if (resources.vertexArrayId != 0)
		renderDevice->DestroyVertexArrays(1, &resources.vertexArrayId);

	if (resources.textureId != 0)
		renderDevice->DestroyTextures(1, &resources.textureId);
}

void TerrainSystem::Reallocate(size_t required)
{
	if (required <= data.allocated)
		return;

	required = static_cast<unsigned int>(Math::UpperPowerOfTwo(required));

	// Reserve same amount in entity map
	entityMap.Reserve(required);

	InstanceData newData;
	size_t bytes = required * (sizeof(Entity) + sizeof(TerrainInstance));

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
	uniforms.terrainResolution = static_cast<float>(params.terrainResolution);
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

	if (heightMap != nullptr)
	{
		renderDevice->SetUniformInt(heightMap->uniformLocation, 0);
		renderDevice->SetActiveTextureUnit(0);
		renderDevice->BindTexture(RenderTextureTarget::Texture2d, resources.textureId);
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

	// Bind material uniform block to shader
	renderDevice->BindBufferBase(RenderBufferTarget::UniformBuffer, UniformBlockBinding::Material, material.uniformBufferObject);

	const MeshDrawData* draw = meshManager->GetDrawData(resources.meshId);
	renderDevice->BindVertexArray(draw->vertexArrayObject);
	renderDevice->DrawIndexed(draw->primitiveMode, draw->count, draw->indexType);
}
