#include "Rendering/TerrainInstance.hpp"

#include <cmath>

#include "Debug/Debug.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/RenderDevice.hpp"
#include "Rendering/StaticUniformBuffer.hpp"
#include "Rendering/VertexFormat.hpp"
#include "Rendering/RenderViewport.hpp"

#include "Resources/MeshManager.hpp"
#include "Resources/MaterialManager.hpp"
#include "Resources/ShaderManager.hpp"

#include "System/IncludeOpenGL.hpp"

TerrainInstance::TerrainInstance(
	Allocator* allocator,
	RenderDevice* renderDevice,
	MeshManager* meshManager,
	ShaderManager* shaderManager) :
	allocator(allocator),
	renderDevice(renderDevice),
	meshManager(meshManager),
	shaderManager(shaderManager),
	heightValues(allocator),
	terrainSize(64.0f),
	terrainResolution(64),
	minHeight(-2.0f),
	maxHeight(0.0f),
	heightData(nullptr),
	vertexArrayId(0),
	uniformBufferId(0),
	textureId(0)
{
	meshId = MeshId{ 0 };
}

TerrainInstance::~TerrainInstance()
{
	allocator->Deallocate(heightData);

	if (vertexArrayId != 0)
		renderDevice->DestroyVertexArrays(1, &vertexArrayId);

	if (textureId != 0)
		renderDevice->DestroyTextures(1, &textureId);
}

void TerrainInstance::Initialize()
{
	// Create texture data

	size_t texSize = terrainResolution;
	float texSizeInv = 1.0f / texSize;
	float scale = 16.0f;
	size_t dataSizeBytes = texSize * texSize * sizeof(uint16_t);
	heightData = static_cast<uint16_t*>(allocator->Allocate(dataSizeBytes));

	for (size_t y = 0; y < texSize; ++y)
	{
		for (size_t x = 0; x < texSize; ++x)
		{
			size_t pixelIndex = y * texSize + x;
			float normalized = 0.5f + std::sin(x * texSizeInv * scale) * 0.25f + std::sin(y * texSizeInv * scale) * 0.25f;
			uint16_t height = static_cast<uint16_t>(normalized * UINT16_MAX);
			heightData[pixelIndex] = height;
		}
	}

	renderDevice->CreateTextures(1, &textureId);
	renderDevice->BindTexture(RenderTextureTarget::Texture2d, textureId);

	RenderCommandData::SetTextureStorage2D storage{
		RenderTextureTarget::Texture2d, 1, RenderTextureSizedFormat::R16, texSize, texSize
	};
	renderDevice->SetTextureStorage2D(&storage);

	RenderCommandData::SetTextureSubImage2D subimage{
		RenderTextureTarget::Texture2d, 0, 0, 0, texSize, texSize, RenderTextureBaseFormat::R,
		RenderTextureDataType::UnsignedShort, heightData
	};
	renderDevice->SetTextureSubImage2D(&subimage);

	// Create vertex data

	size_t sideVerts = terrainResolution + 1;
	size_t vertCount = sideVerts * sideVerts;
	size_t vertexComponents = 2;
	size_t vertSize = sizeof(float) * vertexComponents;
	float* vertexData = static_cast<float*>(allocator->Allocate(vertCount * vertSize));

	size_t sideQuads = terrainResolution;
	size_t quadIndices = 3 * 2; // 3 indices per triangle, 2 triangles per quad
	size_t indexCount = sideQuads * sideQuads * quadIndices;
	unsigned short* indexData = static_cast<unsigned short*>(allocator->Allocate(indexCount * sizeof(unsigned short)));

	float origin = terrainSize * -0.5f;
	float quadSize = terrainSize / terrainResolution;

	// Set vertex data
	for (size_t x = 0; x < sideVerts; ++x)
	{
		for (size_t y = 0; y < sideVerts; ++y)
		{
			size_t vertIndex = y * sideVerts + x;
			vertexData[vertIndex * vertexComponents + 0] = origin + x * quadSize;
			vertexData[vertIndex * vertexComponents + 1] = origin + y * quadSize;
		}
	}

	// Set index data
	for (size_t x = 0; x < sideQuads; ++x)
	{
		for (size_t y = 0; y < sideQuads; ++y)
		{
			size_t quad = y * sideQuads + x;
			indexData[quad * quadIndices + 0] = y * sideVerts + x;
			indexData[quad * quadIndices + 1] = (y + 1) * sideVerts + x;
			indexData[quad * quadIndices + 2] = y * sideVerts + (x + 1);
			indexData[quad * quadIndices + 3] = (y + 1) * sideVerts + x;
			indexData[quad * quadIndices + 4] = (y + 1) * sideVerts + (x + 1);
			indexData[quad * quadIndices + 5] = y * sideVerts + (x + 1);
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
	
	meshId = meshManager->CreateMesh();
	meshManager->UploadIndexed(meshId, data);

	BoundingBox bounds;
	bounds.center = Vec3f(0.0f, 0.0f, 0.0f);
	bounds.extents = Vec3f(terrainSize * 0.5f, terrainSize * 0.5f, terrainSize * 0.5f);
	meshManager->SetBoundingBox(meshId, bounds);

	allocator->Deallocate(indexData);
	allocator->Deallocate(vertexData);

	// Create uniform buffer

	renderDevice->CreateBuffers(1, &uniformBufferId);
	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, uniformBufferId);
	renderDevice->SetBufferData(RenderBufferTarget::UniformBuffer, sizeof(UniformBlock), nullptr, RenderBufferUsage::DynamicDraw);
}

void TerrainInstance::RenderTerrain(const MaterialData& material, const RenderViewport& viewport)
{
	UniformBlock uniforms;
	uniforms.MVP = viewport.viewProjection;
	uniforms.MV = viewport.view;
	uniforms.textureScale = Vec2f(0.25f, 0.25f);
	uniforms.terrainSize = terrainSize;
	uniforms.terrainResolution = terrainResolution;
	uniforms.minHeight = minHeight;
	uniforms.maxHeight = maxHeight;

	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, uniformBufferId);
	renderDevice->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, sizeof(UniformBlock), &uniforms);

	// Bind object transform uniform block to shader
	renderDevice->BindBufferBase(RenderBufferTarget::UniformBuffer, TransformUniformBlock::BindingPoint, uniformBufferId);

	renderDevice->UseShaderProgram(material.cachedShaderDeviceId);

	const TextureUniform* heightMap = material.uniforms.FindTextureUniformByNameHash("height_map"_hash);
	const TextureUniform* albedoMap = material.uniforms.FindTextureUniformByNameHash("albedo_map"_hash);

	if (heightMap != nullptr)
	{
		renderDevice->SetUniformInt(heightMap->uniformLocation, 0);
		renderDevice->SetActiveTextureUnit(0);
		renderDevice->BindTexture(RenderTextureTarget::Texture2d, textureId);
	}

	if (albedoMap != nullptr)
	{
		renderDevice->SetUniformInt(albedoMap->uniformLocation, 1);
		renderDevice->SetActiveTextureUnit(1);
		renderDevice->BindTexture(RenderTextureTarget::Texture2d, albedoMap->textureName);
	}

	// Bind material uniform block to shader
	renderDevice->BindBufferBase(RenderBufferTarget::UniformBuffer, MaterialUniformBlock::BindingPoint, material.uniformBufferObject);

	MeshDrawData* draw = meshManager->GetDrawData(meshId);
	renderDevice->BindVertexArray(draw->vertexArrayObject);
	renderDevice->DrawIndexed(draw->primitiveMode, draw->count, draw->indexType);
}
