#pragma once

#include "Vec3.hpp"
#include "Mat4x4.hpp"
#include "Color.hpp"
#include "ObjectId.hpp"

struct Camera;

class DebugVectorRenderer
{
private:
	enum class PrimitiveType
	{
		Line,
		Cube,
		Sphere
	};

	struct Primitive
	{
		PrimitiveType type;
		Mat4x4f transform;
		Color color;
	};

	Primitive* primitives;
	unsigned int primitiveCount;
	unsigned int primitiveAllocated;

	ObjectId meshIds[3];

	void CreateMeshes();

public:
	DebugVectorRenderer();
	~DebugVectorRenderer();

	void DrawLine(const Vec3f& start, const Vec3f& end, const Color& color);
	void DrawCube(const Mat4x4f& transform, const Color& color);
	void DrawSphere(const Vec3f& position, float radius, const Color& color);

	void Render(const Camera& camera);
};
