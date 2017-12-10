#pragma once

#include "Vec2.hpp"
#include "Vec3.hpp"
#include "Mat4x4.hpp"
#include "Color.hpp"
#include "ObjectId.hpp"

class Camera;

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
		bool screenSpace;
		PrimitiveType type;
		Mat4x4f transform;
		Color color;
	};

	Primitive* primitives;
	unsigned int primitiveCount;
	unsigned int primitiveAllocated;

	bool meshesInitialized;
	unsigned int meshIds[3];

	Camera* activeCamera;

	void CreateMeshes();

public:
	DebugVectorRenderer();
	~DebugVectorRenderer();

	void SetActiveCamera(Camera* camera) { this->activeCamera = camera; }

	void DrawLineScreen(const Vec2f& start, const Vec2f& end, const Color& color);

	void DrawLine(const Vec3f& start, const Vec3f& end, const Color& color);
	void DrawCube(const Mat4x4f& transform, const Color& color);
	void DrawSphere(const Vec3f& position, float radius, const Color& color);

	void Render();
};
