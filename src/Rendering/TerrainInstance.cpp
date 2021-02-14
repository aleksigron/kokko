#include "Rendering/TerrainInstance.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/RenderDevice.hpp"
#include "Rendering/StaticUniformBuffer.hpp"
#include "Rendering/VertexFormat.hpp"

#include "Resources/MeshManager.hpp"
#include "Resources/MaterialManager.hpp"

TerrainInstance::TerrainInstance(
	Allocator* allocator,
	RenderDevice* renderDevice,
	MeshManager* meshManager) :
	allocator(allocator),
	renderDevice(renderDevice),
	meshManager(meshManager),
	heightValues(allocator),
	terrainSize(32.0f),
	terrainResolution(32),
	vertexArray(0)
{
}

TerrainInstance::~TerrainInstance()
{
	if (vertexArray != 0)
		renderDevice->DestroyVertexArrays(1, &vertexArray);
}

void TerrainInstance::Initialize()
{
	size_t sideVerts = terrainResolution + 1;
	size_t vertCount = sideVerts * sideVerts;
	size_t vertexComponents = 2;
	size_t vertSize = sizeof(float) * vertexComponents;
	float* vertexData = static_cast<float*>(allocator->Allocate(vertCount * vertSize));

	size_t sideQuads = terrainResolution;
	size_t quadIndices = 3 * 2; // 3 indices per triangle, 2 triangles per quad
	size_t indexCount = sideQuads * sideQuads * quadIndices;
	unsigned short* indexData = static_cast<unsigned short*>(allocator->Allocate(indexCount * sizeof(unsigned short)));

	float quadSize = terrainSize / terrainResolution;

	// Set vertex data
	for (size_t x = 0; x < sideVerts; ++x)
	{
		for (size_t y = 0; y < sideVerts; ++y)
		{
			size_t vertIndex = y * sideVerts + x;
			vertexData[vertIndex * vertexComponents + 0] = x * quadSize;
			vertexData[vertIndex * vertexComponents + 1] = y * quadSize;
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
}


void TerrainInstance::RenderTerrain(const MaterialData& material)
{
	renderDevice->UseShaderProgram(material.cachedShaderDeviceId);

	// Bind material uniform block to shader
	renderDevice->BindBufferBase(RenderBufferTarget::UniformBuffer, MaterialUniformBlock::BindingPoint, material.uniformBufferObject);


	MeshDrawData* draw = meshManager->GetDrawData(meshId);
	renderDevice->BindVertexArray(draw->vertexArrayObject);
	renderDevice->DrawIndexed(draw->primitiveMode, draw->count, draw->indexType);
}
