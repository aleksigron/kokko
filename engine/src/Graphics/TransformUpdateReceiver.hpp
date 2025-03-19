#pragma once

#include <cstddef>

namespace kokko
{

struct Entity;
struct Mat4x4f;

class TransformUpdateReceiver
{
public:
	virtual void NotifyUpdatedTransforms(size_t count, const Entity* entities, const Mat4x4f* transforms) = 0;
};

} // namespace kokko
