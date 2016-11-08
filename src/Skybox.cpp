#include "Skybox.hpp"

#include "Mat4x4.hpp"

#include "Engine.hpp"
#include "Scene.hpp"
#include "SceneLayer.hpp"
#include "Renderer.hpp"
#include "ResourceManager.hpp"

#include "Mesh.hpp"
#include "VertexFormat.hpp"

Skybox::Skybox() :
	renderScene(nullptr),
	renderObjectId(0),
	sceneObjectId(0)
{
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
	ResourceManager* resourceManager = engine->GetResourceManager();

	this->renderScene = scene;

	ObjectId meshId = resourceManager->meshes.Add();
	Mesh& mesh = resourceManager->meshes.Get(meshId);

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

	// Add scene object
	sceneObjectId = scene->AddSceneObject();

	// Add render object
	renderObjectId = renderer->AddRenderObject();
	RenderObject& renderObject = renderer->GetRenderObject(renderObjectId);
	renderObject.materialId = materialId;
	renderObject.meshId = meshId;
	renderObject.sceneObjectId = sceneObjectId;
	renderObject.layer = SceneLayer::Skybox;
}

void Skybox::UpdateTransform(const Vec3f& cameraPosition) const
{
	Mat4x4f skyboxTransform = Mat4x4f::Translate(cameraPosition);
	this->renderScene->SetLocalTransform(this->sceneObjectId, skyboxTransform);
}
