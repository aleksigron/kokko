#include "Resources/Skybox.hpp"

#include "Math/Mat4x4.hpp"

#include "Engine/Engine.hpp"
#include "Scene/Scene.hpp"
#include "Scene/SceneLayer.hpp"
#include "Scene/SceneManager.hpp"
#include "Rendering/Renderer.hpp"
#include "Resources/MeshManager.hpp"
#include "Resources/MaterialManager.hpp"
#include "Entity/EntityManager.hpp"
#include "Resources/ResourceManager.hpp"

#include "Rendering/VertexFormat.hpp"

Skybox::Skybox() :
	renderSceneId(0)
{
	entity = Entity{};
}

Skybox::~Skybox()
{
}

void Skybox::Initialize(Scene* scene, MaterialId materialId)
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
	MaterialManager* materialManager = engine->GetMaterialManager();
	MeshManager* meshManager = engine->GetMeshManager();

	MeshId meshId = meshManager->CreateMesh();

	IndexedVertexData<Vertex3f, unsigned short> data;
	data.primitiveMode = MeshPrimitiveMode::Triangles;
	data.vertData = vertexData;
	data.vertCount = sizeof(vertexData) / sizeof(Vertex3f);
	data.idxData = indexData;
	data.idxCount = sizeof(indexData) / sizeof(unsigned short);

	meshManager->Upload_3f(meshId, data);

	BoundingBox bounds;
	bounds.center = Vec3f(0.0f, 0.0f, 0.0f);
	bounds.extents = Vec3f(0.5f, 0.5f, 0.5f);
	meshManager->SetBoundingBox(meshId, bounds);

	this->renderSceneId = scene->GetSceneId();
	this->entity = entityManager->Create();

	// Add scene object
	scene->AddSceneObject(this->entity);

	// Add render object
	RenderObjectId renderObjectId = renderer->AddRenderObject(this->entity);
	renderer->SetMeshId(renderObjectId, meshId);

	RenderOrderData order;
	order.material = materialId;
	order.transparency = materialManager->GetTransparency(materialId);
	renderer->SetOrderData(renderObjectId, order);
}

void Skybox::UpdateTransform(const Vec3f& cameraPosition) const
{
	Mat4x4f skyboxTransform = Mat4x4f::Translate(cameraPosition);

	Scene* scene = Engine::GetInstance()->GetSceneManager()->GetScene(renderSceneId);
	SceneObjectId sceneObject = scene->Lookup(entity);
	scene->SetLocalTransform(sceneObject, skyboxTransform);
}
