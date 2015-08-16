#pragma once

#include "Vec3.h"
#include "Mat4x4.h"

#include "Matrix.h"

class Transform
{
public:
	Vec3f position;
	Mat4x4f rotation;
	Vec3f scale = Vec3f(1.0f, 1.0f, 1.0f);
	
	inline Mat4x4f GetTransformMatrix() const
	{
		return Matrix::Translate(this->position) * this->rotation * Matrix::Scale(this->scale);
	}
};