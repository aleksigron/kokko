#pragma once

#include "Math/Vec2.hpp"
#include "Math/Vec3.hpp"
#include "Math/Mat4x4.hpp"
#include "Math/Rectangle.hpp"
#include "Core/Color.hpp"
#include "Resources/MeshData.hpp"
#include "Math/Projection.hpp"

class Allocator;
class Camera;
class RenderDevice;

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

	Allocator* allocator;

	RenderDevice* renderDevice;

	Primitive* primitives;
	unsigned int primitiveCount;
	unsigned int primitiveAllocated;

	bool meshesInitialized;
	MeshId meshIds[4];

	void CreateMeshes();

public:
	DebugVectorRenderer(Allocator* allocator, RenderDevice* renderDevice);
	~DebugVectorRenderer();

	void DrawLineScreen(const Vec2f& start, const Vec2f& end, const Color& color);
	void DrawLine(const Vec3f& start, const Vec3f& end, const Color& color);

	void DrawWireCube(const Mat4x4f& transform, const Color& color);

	void DrawWireSphere(const Vec3f& position, float radius, const Color& color);

	void DrawRectangleScreen(const Rectanglef& rectangle, const Color& color);

	void DrawWireFrustum(const Mat4x4f& transform, const ProjectionParameters& projection, const Color& color);

	void Render(Camera* camera);
};
