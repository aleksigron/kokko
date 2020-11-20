#include "Resources/MeshPresets.hpp"

#include "Resources/MeshManager.hpp"

#include "Rendering/VertexFormat.hpp"

void MeshPresets::UploadCube(MeshManager* meshManager, MeshId meshId)
{
	static const Vertex3f vertexData[] = {
		Vertex3f{ Vec3f(-0.5f, -0.5f, -0.5f) },
		Vertex3f{ Vec3f(0.5f, -0.5f, -0.5f) },
		Vertex3f{ Vec3f(-0.5f, -0.5f, 0.5f) },
		Vertex3f{ Vec3f(0.5f, -0.5f, 0.5f) },
		Vertex3f{ Vec3f(-0.5f, 0.5f, -0.5f) },
		Vertex3f{ Vec3f(0.5f, 0.5f, -0.5f) },
		Vertex3f{ Vec3f(-0.5f, 0.5f, 0.5f) },
		Vertex3f{ Vec3f(0.5f, 0.5f, 0.5f) }
	};

	static const unsigned short indexData[] = {
		0, 5, 4, 0, 1, 5,
		4, 7, 6, 4, 5, 7,
		5, 3, 7, 5, 1, 3,
		2, 1, 0, 2, 3, 1,
		0, 6, 2, 0, 4, 6,
		3, 6, 7, 3, 2, 6
	};

	IndexedVertexData<Vertex3f, unsigned short> data;
	data.primitiveMode = MeshPrimitiveMode::Triangles;
	data.vertData = vertexData;
	data.vertCount = sizeof(vertexData) / sizeof(Vertex3f);
	data.idxData = indexData;
	data.idxCount = sizeof(indexData) / sizeof(unsigned short);

	meshManager->UploadIndexed_3f(meshId, data, RenderData::BufferUsage::StaticDraw);

	BoundingBox bounds;
	bounds.center = Vec3f(0.0f, 0.0f, 0.0f);
	bounds.extents = Vec3f(0.5f, 0.5f, 0.5f);
	meshManager->SetBoundingBox(meshId, bounds);
}

void MeshPresets::UploadPlane(MeshManager* meshManager, MeshId meshId)
{
	static const Vertex3f vertexData[] = {
		Vertex3f{ Vec3f(-1.0f, -1.0f, 0.0f) },
		Vertex3f{ Vec3f(1.0f, -1.0f, 0.0f) },
		Vertex3f{ Vec3f(-1.0f, 1.0f, 0.0f) },
		Vertex3f{ Vec3f(1.0f, 1.0f, 0.0f) }
	};

	static const unsigned short indexData[] = { 0, 1, 2, 1, 3, 2 };

	IndexedVertexData<Vertex3f, unsigned short> data;
	data.primitiveMode = MeshPrimitiveMode::Triangles;
	data.vertData = vertexData;
	data.vertCount = sizeof(vertexData) / sizeof(Vertex3f);
	data.idxData = indexData;
	data.idxCount = sizeof(indexData) / sizeof(unsigned short);

	meshManager->UploadIndexed_3f(meshId, data, RenderData::BufferUsage::StaticDraw);
}
