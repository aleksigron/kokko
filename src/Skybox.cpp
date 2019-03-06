#include "Skybox.hpp"

#include "Mat4x4.hpp"

#include "Engine.hpp"
#include "Scene.hpp"
#include "SceneLayer.hpp"
#include "SceneManager.hpp"
#include "Renderer.hpp"
#include "EntityManager.hpp"
#include "ResourceManager.hpp"

#include "Mesh.hpp"
#include "VertexFormat.hpp"

Skybox::Skybox() :
	renderSceneId(0),
	renderObjectId(0)
{
	entity = Entity{};
}

Skybox::~Skybox()
{
}

void Skybox::Initialize(Scene* scene, unsigned int materialId)
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

	Engine* engine = Engine::GetInstance();
	Renderer* renderer = engine->GetRenderer();
	EntityManager* entityManager = engine->GetEntityManager();
	ResourceManager* resourceManager = engine->GetResourceManager();

	unsigned meshId = resourceManager->CreateMesh();
	Mesh& mesh = resourceManager->GetMesh(meshId);

	BufferRef<Vertex3f> vertices;
	vertices.data = vertexData;
	vertices.count = sizeof(vertexData) / sizeof(Vertex3f);

	BufferRef<unsigned short> indices;
	indices.data = indexData;
	indices.count = sizeof(indexData) / sizeof(unsigned short);

	mesh.bounds.center = Vec3f(0.0f, 0.0f, 0.0f);
	mesh.bounds.extents = Vec3f(0.5f, 0.5f, 0.5f);
	mesh.SetPrimitiveMode(Mesh::PrimitiveMode::Triangles);
	mesh.Upload_3f(vertices, indices);

	this->renderSceneId = scene->GetSceneId();
	this->entity = entityManager->Create();

	// Add scene object
	scene->AddSceneObject(this->entity);

	// Add render object
	renderObjectId = renderer->AddRenderObject();
	RenderObject& renderObject = renderer->GetRenderObject(renderObjectId);
	renderObject.materialId = materialId;
	renderObject.meshId = meshId;
	renderObject.entity = this->entity;
	renderObject.layer = SceneLayer::Skybox;
}

void Skybox::UpdateTransform(const Vec3f& cameraPosition) const
{
	Mat4x4f skyboxTransform = Mat4x4f::Translate(cameraPosition);

	Scene* scene = Engine::GetInstance()->GetSceneManager()->GetScene(renderSceneId);
	SceneObjectId sceneObject = scene->Lookup(entity);
	scene->SetLocalTransform(sceneObject, skyboxTransform);
}
