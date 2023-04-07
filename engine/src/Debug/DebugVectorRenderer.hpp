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

namespace kokko
{
namespace render
{
class CommandEncoder;
}
}

struct Entity;
struct CameraParameters;
class Allocator;
class RenderDevice;
class MeshManager;
class ShaderManager;
class World;
class Window;

class DebugVectorRenderer
{
private:
	enum class PrimitiveType
	{
		// Static
		Line,
		WireCube,
		WireSphere,
		Rectangle,

		// Dynamic
		LineChain
	};

	struct Primitive
	{
		Mat4x4f transform;
		Color color;
		int dynamicMeshIndex;
		PrimitiveType type;
		bool screenSpace;
	};

	Allocator* allocator;
	RenderDevice* renderDevice;
	MeshManager* meshManager;
	ShaderManager* shaderManager;

	Primitive* primitives;
	unsigned int primitiveCount;
	unsigned int primitiveAllocated;

	struct DynamicMesh
	{
		MeshId meshId;
		size_t bufferSize;
		bool used;
	};

	DynamicMesh* dynamicMeshes;
	unsigned int dynamicMeshCount;
	unsigned int dynamicMeshAllocated;

	bool meshesInitialized;
	MeshId staticMeshes[4];

	ShaderId shaderId;

	enum UniformBufferType { ObjectBuffer, MaterialBuffer };

	bool buffersInitialized;
	kokko::RenderBufferId uniformBufferIds[2];

	DynamicMesh* GetDynamicMesh(size_t byteSize);

public:
	DebugVectorRenderer(Allocator* allocator, RenderDevice* renderDevice);
	~DebugVectorRenderer();

	void Initialize(MeshManager* meshManager, ShaderManager* shaderManager);
	void Deinitialize();

	void DrawLineScreen(const Vec2f& start, const Vec2f& end, const Color& color);
	void DrawLineChainScreen(size_t count, const Vec3f* points, const Color& color);
	void DrawLine(const Vec3f& start, const Vec3f& end, const Color& color);

	void DrawWireCube(const Mat4x4f& transform, const Color& color);

	void DrawWireSphere(const Vec3f& position, float radius, const Color& color);

	void DrawRectangleScreen(const Rectanglef& rectangle, const Color& color);

	void DrawWireFrustum(const Mat4x4f& transform, const ProjectionParameters& projection, const Color& color);

	void Render(kokko::render::CommandEncoder* encoder, World* world, const ViewRectangle& viewport, const Optional<CameraParameters>& editorCamera);
};
