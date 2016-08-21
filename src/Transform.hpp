#pragma once

#include "Vec3.hpp"
#include "Mat3x3.hpp"
#include "Mat4x4.hpp"

class TransformSource
{
public:
	Vec3f position;
	Mat3x3f rotation;
	Vec3f scale = Vec3f(1.0f, 1.0f, 1.0f);
	
	inline Mat4x4f GetTransformMatrix() const
	{
		return Mat4x4f::Translate(position) * Mat4x4f(rotation) * Mat4x4f::Scale(scale);
	}

	inline Vec3f Forward() const
	{ return Vec3f(-rotation[6], -rotation[7], -rotation[8]); }

	inline Vec3f Right() const
	{ return Vec3f(rotation[0], rotation[1], rotation[2]); }

	inline Vec3f Up() const
	{ return Vec3f(rotation[3], rotation[4], rotation[5]); }
};
