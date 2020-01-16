#pragma once

#include "Math/Mat4x4.hpp"
#include "Entity.hpp"
#include "Math/Frustum.hpp"

class Scene;

class Camera
{
private:
	Entity entity;

public:
	ProjectionParameters parameters;

	void SetEntity(Entity e) { entity = e; }
	Entity GetEntity() const { return entity; }

	static Mat4x4f GetViewMatrix(const Mat4x4f& m)
	{
		Mat3x3f inverseRotation = m.Get3x3().GetTransposed();
		Vec3f translation = -(inverseRotation * Vec3f(m[12], m[13], m[14]));

		Mat4x4f v(inverseRotation);
		v[12] = translation.x;
		v[13] = translation.y;
		v[14] = translation.z;

		return v;
	}
};
