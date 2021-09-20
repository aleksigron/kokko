#pragma once

#include <cstddef>

struct Entity;
struct Mat4x4f;

class TransformUpdateReceiver
{
public:
	virtual void NotifyUpdatedTransforms(size_t count, const Entity* entities, const Mat4x4f* transforms) = 0;
};
