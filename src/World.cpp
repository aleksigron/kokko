#include "World.hpp"

#include "Mesh.hpp"
#include "VertexFormat.hpp"

#include "ResourceManager.hpp"

World::World()
{
}

World::~World()
{
}

void World::InitializeSkyboxMesh(ResourceManager* resourceManager)
{
	static Vertex3f vertexData[] = {
		Vertex3f{ Vec3f(-0.5f, -0.5f, -0.5f) },
		Vertex3f{ Vec3f(0.5f, -0.5f, -0.5f) },
		Vertex3f{ Vec3f(-0.5f, -0.5f, 0.5f) },
		Vertex3f{ Vec3f(0.5f, -0.5f, 0.5f) },
		Vertex3f{ Vec3f(-0.5f, 0.5f, -0.5f) },
		Vertex3f{ Vec3f(0.5f, 0.5f, -0.5f) },
		Vertex3f{ Vec3f(-0.5f, 0.5f, 0.5f) },
		Vertex3f{ Vec3f(0.5f, 0.5f, 0.5f) }
	};

	static unsigned short indexData[] = {
		0, 5, 4, 0, 1, 5,
		4, 7, 6, 4, 5, 7,
		5, 3, 7, 5, 1, 3,
		2, 1, 0, 2, 3, 1,
		0, 6, 2, 0, 4, 6,
		3, 6, 7, 3, 2, 6
	};

	skyboxMeshId = resourceManager->meshes.Add();
	Mesh& mesh = resourceManager->meshes.Get(skyboxMeshId);

	BufferRef<Vertex3f> vertices;
	vertices.data = vertexData;
	vertices.count = sizeof(vertexData) / sizeof(Vertex3f);

	BufferRef<unsigned short> indices;
	indices.data = indexData;
	indices.count = sizeof(indexData) / sizeof(unsigned short);

	mesh.SetPrimitiveMode(Mesh::PrimitiveMode::Triangles);
	mesh.Upload_3f(vertices, indices);
}
