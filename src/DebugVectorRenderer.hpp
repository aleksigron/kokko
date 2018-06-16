#pragma once

#include "Vec2.hpp"
#include "Vec3.hpp"
#include "Mat4x4.hpp"
#include "Rectangle.hpp"
#include "Color.hpp"
#include "ObjectId.hpp"

class Camera;

class DebugVectorRenderer
{
private:
	enum class PrimitiveType
	{
		Line,
		WireCube,
		WireSphere,
		Rectangle
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
	unsigned int meshIds[4];

	Camera* activeCamera;

	void CreateMeshes();

public:
	DebugVectorRenderer();
	~DebugVectorRenderer();

	void SetActiveCamera(Camera* camera) { this->activeCamera = camera; }

	void DrawLineScreen(const Vec2f& start, const Vec2f& end, const Color& color);
	void DrawLine(const Vec3f& start, const Vec3f& end, const Color& color);

	void DrawWireCube(const Mat4x4f& transform, const Color& color);

	void DrawWireSphere(const Vec3f& position, float radius, const Color& color);

	void DrawRectangleScreen(const Rectangle& rectangle, const Color& color);

	void Render();
};
