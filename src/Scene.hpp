#pragma once

#include "Mat4x4.hpp"
#include "Color.hpp"
#include "Skybox.hpp"

using SceneObjectId = unsigned int;

struct SceneObjectBatch
{
	static const unsigned int BatchSize = 512;

	unsigned int used;

	SceneObjectId parentIds[BatchSize];
	Mat4x4f localTransforms[BatchSize];
	Mat4x4f worldTransforms[BatchSize];
};

class Scene
{
public:
	static const unsigned int Root = 0;

private:
	SceneObjectBatch objectBatch;

public:
	Scene();
	~Scene();

	Color backgroundColor;
	Skybox skybox;

	SceneObjectId AddSceneObject();

	void SetParent(SceneObjectId object, SceneObjectId parent);
	void SetLocalTransform(SceneObjectId object, const Mat4x4f& transform);

	Mat4x4f& GetWorldTransform(SceneObjectId object)
	{
		return objectBatch.worldTransforms[object];
	}
	
	Mat4x4f& GetLocalTransform(SceneObjectId object)
	{
		return objectBatch.localTransforms[object];
	}

	void CalculateWorldTransforms();
};
