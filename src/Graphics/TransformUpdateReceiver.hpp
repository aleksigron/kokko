#pragma once

struct Entity;
struct Mat4x4f;

class TransformUpdateReceiver
{
public:
	virtual void NotifyUpdatedTransforms(unsigned int count, const Entity* entities, const Mat4x4f* transforms) = 0;
};
