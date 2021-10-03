#include "Graphics/TerrainSystem.hpp"

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
	ComponentSystemDefaultImpl(allocator),
	allocator(allocator),
	renderDevice(renderDevice),
	meshManager(meshManager),
	materialManager(materialManager),
	shaderManager(shaderManager)
{
	terrainMaterial = MaterialId::Null;
}

TerrainSystem::~TerrainSystem()
{
	// TODO: Release memory for terrain instances
}

void TerrainSystem::Initialize()
{
	KOKKO_PROFILE_FUNCTION();

	StringRef path("engine/materials/deferred_geometry/terrain.material.json");
	terrainMaterial = materialManager->GetIdByPath(path);
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

	for (auto& terrainComponent : *this)
	{
		RenderTerrain(terrainComponent.data, material, *params.viewport);
	}
}

void TerrainSystem::InitializeTerrain(TerrainId id)
{
	KOKKO_PROFILE_FUNCTION();

	TerrainInstance terrain = GetData(id);

	// Create texture data

	int texSize = static_cast<int>(terrain.terrainResolution);
	float texSizeInv = 1.0f / texSize;
	float scale = 64.0f;
	size_t dataSizeBytes = texSize * texSize * sizeof(uint16_t);
	terrain.heightData = static_cast<uint16_t*>(allocator->Allocate(dataSizeBytes));

	for (size_t y = 0; y < texSize; ++y)
	{
		for (size_t x = 0; x < texSize; ++x)
		{
			size_t pixelIndex = y * texSize + x;
			float normalized = 0.5f + std::sin(x * texSizeInv * scale) * 0.25f + std::sin(y * texSizeInv * scale) * 0.25f;
			uint16_t height = static_cast<uint16_t>(normalized * UINT16_MAX);
			terrain.heightData[pixelIndex] = height;
		}
	}

	renderDevice->CreateTextures(1, &terrain.textureId);
	renderDevice->BindTexture(RenderTextureTarget::Texture2d, terrain.textureId);

	RenderCommandData::SetTextureStorage2D textureStorage{
		RenderTextureTarget::Texture2d, 1, RenderTextureSizedFormat::R16, texSize, texSize
	};
	renderDevice->SetTextureStorage2D(&textureStorage);

	RenderCommandData::SetTextureSubImage2D subimage{
		RenderTextureTarget::Texture2d, 0, 0, 0, texSize, texSize, RenderTextureBaseFormat::R,
		RenderTextureDataType::UnsignedShort, terrain.heightData
	};
	renderDevice->SetTextureSubImage2D(&subimage);

	// Create vertex data

	int sideVerts = terrain.terrainResolution + 1;
	unsigned int vertCount = sideVerts * sideVerts;
	size_t vertexComponents = 2;
	size_t vertSize = sizeof(float) * vertexComponents;
	float* vertexData = static_cast<float*>(allocator->Allocate(vertCount * vertSize));

	int sideQuads = terrain.terrainResolution;
	int quadIndices = 3 * 2; // 3 indices per triangle, 2 triangles per quad
	unsigned int indexCount = sideQuads * sideQuads * quadIndices;
	uint16_t* indexData = static_cast<uint16_t*>(allocator->Allocate(indexCount * sizeof(uint16_t)));
	// TODO: Use 32-bit index type if necessary

	float quadSize = 1.0f / terrain.terrainSize;

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

	IndexedVertexData data;
	data.vertexFormat = vertexFormatPos;
	data.primitiveMode = RenderPrimitiveMode::Triangles;
	data.vertexData = vertexData;
	data.vertexCount = vertCount;
	data.indexData = indexData;
	data.indexCount = indexCount;

	terrain.meshId = meshManager->CreateMesh();
	meshManager->UploadIndexed(terrain.meshId, data);

	float halfTerrainSize = terrain.terrainSize * 0.5f;

	BoundingBox bounds;
	bounds.center = Vec3f(0.0f, 0.0f, 0.0f);
	bounds.extents = Vec3f(halfTerrainSize, halfTerrainSize, halfTerrainSize);
	meshManager->SetBoundingBox(terrain.meshId, bounds);

	allocator->Deallocate(indexData);
	allocator->Deallocate(vertexData);

	// Create uniform buffer

	renderDevice->CreateBuffers(1, &terrain.uniformBufferId);
	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, terrain.uniformBufferId);

	RenderCommandData::SetBufferStorage bufferStorage{};
	bufferStorage.target = RenderBufferTarget::UniformBuffer;
	bufferStorage.size = sizeof(TerrainUniformBlock);
	bufferStorage.data = nullptr;
	bufferStorage.dynamicStorage = true;
	renderDevice->SetBufferStorage(&bufferStorage);

	// Update terrain instance back into ComponentSystemDefaultImpl
	SetData(id, terrain);
}

void TerrainSystem::DeinitializeTerrain(TerrainId id)
{
	KOKKO_PROFILE_FUNCTION();

	TerrainInstance terrain = GetData(id);

	allocator->Deallocate(terrain.heightData);

	if (terrain.vertexArrayId != 0)
		renderDevice->DestroyVertexArrays(1, &terrain.vertexArrayId);

	if (terrain.textureId != 0)
		renderDevice->DestroyTextures(1, &terrain.textureId);
}

void TerrainSystem::RenderTerrain(TerrainInstance& terrain, const MaterialData& material, const RenderViewport& viewport)
{
	KOKKO_PROFILE_FUNCTION();

	TerrainUniformBlock uniforms;
	uniforms.MVP = viewport.viewProjection;
	uniforms.MV = viewport.view;
	uniforms.textureScale = terrain.textureScale;
	uniforms.terrainSize = terrain.terrainSize;
	uniforms.terrainResolution = static_cast<float>(terrain.terrainResolution);
	uniforms.minHeight = terrain.minHeight;
	uniforms.maxHeight = terrain.maxHeight;

	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, terrain.uniformBufferId);
	renderDevice->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, sizeof(TerrainUniformBlock), &uniforms);

	// Bind object transform uniform block to shader
	renderDevice->BindBufferBase(RenderBufferTarget::UniformBuffer, UniformBlockBinding::Object, terrain.uniformBufferId);

	renderDevice->UseShaderProgram(material.cachedShaderDeviceId);

	const TextureUniform* heightMap = material.uniforms.FindTextureUniformByNameHash("height_map"_hash);
	const TextureUniform* albedoMap = material.uniforms.FindTextureUniformByNameHash("albedo_map"_hash);
	const TextureUniform* roughMap = material.uniforms.FindTextureUniformByNameHash("roughness_map"_hash);

	if (heightMap != nullptr)
	{
		renderDevice->SetUniformInt(heightMap->uniformLocation, 0);
		renderDevice->SetActiveTextureUnit(0);
		renderDevice->BindTexture(RenderTextureTarget::Texture2d, terrain.textureId);
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

	const MeshDrawData* draw = meshManager->GetDrawData(terrain.meshId);
	renderDevice->BindVertexArray(draw->vertexArrayObject);
	renderDevice->DrawIndexed(draw->primitiveMode, draw->count, draw->indexType);
}
