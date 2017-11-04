#pragma once

#include "Mat4x4.hpp"
#include "Color.hpp"
#include "Skybox.hpp"

class Camera;

struct SceneObjectBatch
{
	static const unsigned int BatchSize = 512;

	unsigned int used;

	unsigned int parentIds[BatchSize];
	Mat4x4f localTransforms[BatchSize];
	Mat4x4f worldTransforms[BatchSize];
};

class Scene
{
public:
	static const unsigned int Root = 0;

private:
	SceneObjectBatch* objectBatch;

	Camera* activeCamera;

public:
	Scene();
	~Scene();

	Scene& operator=(Scene&& other);

	Color backgroundColor;
	Skybox skybox;

	unsigned int AddSceneObject();

	void SetParent(unsigned int object, unsigned int parent);
	void SetLocalTransform(unsigned int object, const Mat4x4f& transform);

	Mat4x4f& GetWorldTransform(unsigned int object)
	{
		return objectBatch->worldTransforms[object];
	}
	
	Mat4x4f& GetLocalTransform(unsigned int object)
	{
		return objectBatch->localTransforms[object];
	}

	void CalculateWorldTransforms();

	// Set the camera from which to render the scene
	void SetActiveCamera(Camera* camera);

	Camera* GetActiveCamera() { return activeCamera; }
};
