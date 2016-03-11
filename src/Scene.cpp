#include "Scene.hpp"

Scene::Scene()
{
	// Conceptual root object uses the index 0
	objectBatch.used = 1;

	// Set root object world transform
	objectBatch.worldTransforms[Scene::Root] = Mat4x4f();
}

Scene::~Scene()
{
	
}

SceneObjectId Scene::AddSceneObject()
{
	unsigned int id;

	if (objectBatch.used < SceneObjectBatch::BatchSize)
	{
		id = objectBatch.used++;
	}
	else
	{
		// TODO: Allocate a new batch
		id = 0;
	}

	objectBatch.parentIds[id] = Scene::Root;

	return id;
}

void Scene::SetParent(SceneObjectId object, SceneObjectId parent)
{
	objectBatch.parentIds[object] = parent;
}

void Scene::SetLocalTransform(SceneObjectId object, const Mat4x4f& transform)
{
	objectBatch.localTransforms[object] = transform;
}

void Scene::CalculateWorldTransforms()
{
	Mat4x4f* local = objectBatch.localTransforms;
	Mat4x4f* world = objectBatch.worldTransforms;
	SceneObjectId* parent = objectBatch.parentIds;

	for (unsigned i = 1; i < objectBatch.used; ++i)
	{
		world[i] = world[parent[i]] * local[i];
	}
}
