#include "Resources/MeshPresets.hpp"

#include "Resources/MeshManager.hpp"

#include "Rendering/RenderDeviceEnums.hpp"
#include "Rendering/VertexFormat.hpp"

void MeshPresets::UploadCube(MeshManager* meshManager, MeshId meshId)
{
	static const float vertexData[] = {
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		-0.5f, 0.5f, -0.5f,
		0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, 0.5f
	};

	static const unsigned short indexData[] = {
		0, 5, 4, 0, 1, 5,
		4, 7, 6, 4, 5, 7,
		5, 3, 7, 5, 1, 3,
		2, 1, 0, 2, 3, 1,
		0, 6, 2, 0, 4, 6,
		3, 6, 7, 3, 2, 6
	};

	VertexAttribute vertexAttributes[] = { VertexAttribute::pos3 };
	VertexFormat vertexFormatPos(vertexAttributes, sizeof(vertexAttributes) / sizeof(vertexAttributes[0]));

	IndexedVertexData data;
	data.vertexFormat = vertexFormatPos;
	data.primitiveMode = RenderPrimitiveMode::Triangles;
	data.vertexData = vertexData;
	data.vertexCount = sizeof(vertexData) / (sizeof(float) * 3);
	data.indexData = indexData;
	data.indexCount = sizeof(indexData) / sizeof(unsigned short);

	meshManager->UploadIndexed(meshId, data);

	BoundingBox bounds;
	bounds.center = Vec3f(0.0f, 0.0f, 0.0f);
	bounds.extents = Vec3f(0.5f, 0.5f, 0.5f);
	meshManager->SetBoundingBox(meshId, bounds);
}

void MeshPresets::UploadPlane(MeshManager* meshManager, MeshId meshId)
{
	static const float vertexData[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f
	};

	static const unsigned short indexData[] = { 0, 1, 2, 1, 3, 2 };

	VertexAttribute vertexAttributes[] = { VertexAttribute::pos3 };
	VertexFormat vertexFormatPos(vertexAttributes, sizeof(vertexAttributes) / sizeof(vertexAttributes[0]));

	IndexedVertexData data;
	data.vertexFormat = vertexFormatPos;
	data.vertexData = vertexData;
	data.vertexCount = sizeof(vertexData) / (sizeof(float) * 3);
	data.indexData = indexData;
	data.indexCount = sizeof(indexData) / sizeof(unsigned short);

	meshManager->UploadIndexed(meshId, data);
}
