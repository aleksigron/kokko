#pragma once

#include "Mat4x4.h"
#include "Transform.h"

using SceneObjectId = unsigned int;

struct SceneObjectBatch
{
	static const unsigned int BatchSize = 1 << 9;

	unsigned int used;

	SceneObjectId parentIds[BatchSize];
	Mat4x4f localTransforms[BatchSize];
	Mat4x4f worldTransforms[BatchSize];
	Transform localSources[BatchSize];
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

	SceneObjectId AddSceneObject();

	void SetParent(SceneObjectId object, SceneObjectId parent);
	void SetLocalTransform(SceneObjectId object, const Mat4x4f& transform);

	inline Mat4x4f& GetWorldTransformMatrix(SceneObjectId object)
	{
		return objectBatch.worldTransforms[object];
	}

	inline Transform& GetLocalTransformRef(SceneObjectId object)
	{
		return objectBatch.localSources[object];
	}

	void CalculateWorldTransforms();
};
