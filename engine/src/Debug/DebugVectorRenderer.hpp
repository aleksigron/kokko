#pragma once

#include <cstddef>

#include "Core/Color.hpp"
#include "Core/Optional.hpp"

#include "Math/Vec2.hpp"
#include "Math/Vec3.hpp"
#include "Math/Mat4x4.hpp"
#include "Math/Rectangle.hpp"
#include "Math/Projection.hpp"

#include "Rendering/RenderResourceId.hpp"

#include "Resources/MeshId.hpp"
#include "Resources/ShaderId.hpp"

struct Entity;
struct CameraParameters;
class Allocator;
class Window;

namespace kokko
{

class ModelManager;
class ShaderManager;
class World;

namespace render
{
class CommandEncoder;
class Device;
}

class DebugVectorRenderer
{
private:
	enum class PrimitiveType
	{
		// Static
		Line,
		WireCube,
		WireSphere,
		Rectangle
	};

	struct Primitive
	{
		Mat4x4f transform;
		Color color;
		PrimitiveType type;
		bool screenSpace;
	};

	Allocator* allocator;
	render::Device* renderDevice;
	ModelManager* modelManager;
	ShaderManager* shaderManager;

	Primitive* primitives;
	unsigned int primitiveCount;
	unsigned int primitiveAllocated;

	bool meshesInitialized;
	ModelId staticMeshes[4];

	ShaderId shaderId;

	render::BufferId uniformBufferId;
	uint32_t bufferPrimitivesAllocated;
	uint32_t bufferAlignedSize;

public:
	DebugVectorRenderer(Allocator* allocator, render::Device* renderDevice);
	~DebugVectorRenderer();

	void Initialize(ModelManager* modelManager, ShaderManager* shaderManager);
	void Deinitialize();

	void DrawLineScreen(const Vec2f& start, const Vec2f& end, const Color& color);
	void DrawLine(const Vec3f& start, const Vec3f& end, const Color& color);

	void DrawWireCube(const Mat4x4f& transform, const Color& color);

	void DrawWireSphere(const Vec3f& position, float radius, const Color& color);

	void DrawRectangleScreen(const Rectanglef& rectangle, const Color& color);

	void DrawWireFrustum(const Mat4x4f& transform, const ProjectionParameters& projection, const Color& color);

	void Render(render::CommandEncoder* encoder, World* world, const ViewRectangle& viewport, const Optional<CameraParameters>& editorCamera);
};

} // namespace kokko
