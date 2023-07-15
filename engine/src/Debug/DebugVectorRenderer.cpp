#include "Debug/DebugVectorRenderer.hpp"

#include <cstring>
#include <cmath>

#include "Core/Core.hpp"

#include "Engine/World.hpp"

#include "Graphics/Scene.hpp"

#include "Math/Math.hpp"
#include "Math/Frustum.hpp"

#include "Platform/Window.hpp"

#include "Rendering/CameraParameters.hpp"
#include "Rendering/CameraSystem.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/StaticUniformBuffer.hpp"
#include "Rendering/CommandEncoder.hpp"

#include "Resources/MeshManager.hpp"
#include "Resources/ShaderManager.hpp"

namespace kokko
{

struct DebugVectorBlock
{
	alignas(16) Mat4x4f transform;
	alignas(16) Vec4f color;
};

DebugVectorRenderer::DebugVectorRenderer(
	Allocator* allocator,
	render::Device* renderDevice) :
	allocator(allocator),
	renderDevice(renderDevice),
	shaderManager(nullptr),
	meshManager(nullptr),
	dynamicMeshes(nullptr),
	dynamicMeshCount(0),
	dynamicMeshAllocated(0),
	meshesInitialized(false),
	shaderId(ShaderId{ 0 }),
	bufferPrimitivesAllocated(0),
	bufferAlignedSize(0)
{
	primitiveCount = 0;
	primitiveAllocated = 1024;

	size_t primitivesSize = sizeof(Primitive) * primitiveAllocated;
	primitives = static_cast<Primitive*>(allocator->Allocate(primitivesSize, "DebugVectorRenderer.primitives"));
}

DebugVectorRenderer::~DebugVectorRenderer()
{
}

void DebugVectorRenderer::Initialize(MeshManager* meshManager, ShaderManager* shaderManager)
{
	KOKKO_PROFILE_FUNCTION();

	auto scope = renderDevice->CreateDebugScope(0, ConstStringView("DebugVec_InitResources"));

	int alignment = 0;
	renderDevice->GetIntegerValue(RenderDeviceParameter::UniformBufferOffsetAlignment, &alignment);
	bufferAlignedSize = Math::RoundUpToMultiple(int(sizeof(DebugVectorBlock)), alignment);

	this->meshManager = meshManager;
	this->shaderManager = shaderManager;

	// Initialize shaders

	const char* shaderPath = "engine/shaders/debug/debug_vector.glsl";
	shaderId = shaderManager->FindShaderByPath(ConstStringView(shaderPath));

	// Initialize meshes

	VertexAttribute vertexAttributes[] = { VertexAttribute::pos3 };
	VertexFormat vertexFormatPos(vertexAttributes, sizeof(vertexAttributes) / sizeof(vertexAttributes[0]));
	vertexFormatPos.CalcOffsetsAndSizeInterleaved();

	{
		float lineVertexData[] = {
			0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, -1.0f
		};

		MeshId& lineMeshId = this->staticMeshes[static_cast<unsigned int>(PrimitiveType::Line)];
		lineMeshId = meshManager->CreateMesh();

		VertexData data;
		data.vertexFormat = vertexFormatPos;
		data.primitiveMode = RenderPrimitiveMode::Lines;
		data.vertexData = lineVertexData;
		data.vertexDataSize = sizeof(lineVertexData);
		data.vertexCount = 2;

		meshManager->Upload(lineMeshId, data);
	}

	{
		float cubeVertexData[] = {
			-0.5f, -0.5f, -0.5f,
			0.5f, -0.5f, -0.5f,
			-0.5f, -0.5f, 0.5f,
			0.5f, -0.5f, 0.5f,
			-0.5f, 0.5f, -0.5f,
			0.5f, 0.5f, -0.5f,
			-0.5f, 0.5f, 0.5f,
			0.5f, 0.5f, 0.5f
		};

		unsigned short cubeIndexData[] = {
			0, 1, 1, 3, 3, 2, 2, 0,
			0, 4, 1, 5, 2, 6, 3, 7,
			4, 5, 5, 7, 7, 6, 6, 4
		};

		MeshId& cubeMeshId = this->staticMeshes[static_cast<unsigned int>(PrimitiveType::WireCube)];
		cubeMeshId = meshManager->CreateMesh();

		IndexedVertexData data;
		data.vertexFormat = vertexFormatPos;
		data.primitiveMode = RenderPrimitiveMode::Lines;
		data.vertexData = cubeVertexData;
		data.vertexDataSize = sizeof(cubeVertexData);
		data.vertexCount = sizeof(cubeVertexData) / (sizeof(cubeVertexData[0]) * 3);
		data.indexData = cubeIndexData;
		data.indexDataSize = sizeof(cubeIndexData);
		data.indexCount = sizeof(cubeIndexData) / sizeof(cubeIndexData[0]);

		meshManager->UploadIndexed(cubeMeshId, data);
	}

	{
		static const int sphereVertices = 72;

		float sphereVertexData[sphereVertices * 3];

		for (int i = 0; i < 24; ++i)
		{
			float f = Math::Const::Tau / 24 * i;
			sphereVertexData[i * 3 + 0] = std::sin(f);
			sphereVertexData[i * 3 + 1] = std::cos(f);
			sphereVertexData[i * 3 + 2] = 0.0f;
		}

		for (int i = 0; i < 24; ++i)
		{
			float f = Math::Const::Tau / 24 * i;
			sphereVertexData[24 * 3 + i * 3 + 0] = 0.0f;
			sphereVertexData[24 * 3 + i * 3 + 1] = std::sin(f);
			sphereVertexData[24 * 3 + i * 3 + 2] = std::cos(f);
		}

		for (int i = 0; i < 24; ++i)
		{
			float f = Math::Const::Tau / 24 * i;
			sphereVertexData[48 * 3 + i * 3 + 0] = std::cos(f);
			sphereVertexData[48 * 3 + i * 3 + 1] = 0.0f;
			sphereVertexData[48 * 3 + i * 3 + 2] = std::sin(f);
		}

		unsigned short sphereIndexData[sphereVertices * 2] = {
			0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6,
			6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12,
			12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18,
			18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 0,

			24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30,
			30, 31, 31, 32, 32, 33, 33, 34, 34, 35, 35, 36,
			36, 37, 37, 38, 38, 39, 39, 40, 40, 41, 41, 42,
			42, 43, 43, 44, 44, 45, 45, 46, 46, 47, 47, 24,

			48, 49, 49, 50, 50, 51, 51, 52, 52, 53, 53, 54,
			54, 55, 55, 56, 56, 57, 57, 58, 58, 59, 59, 60,
			60, 61, 61, 62, 62, 63, 63, 64, 64, 65, 65, 66,
			66, 67, 67, 68, 68, 69, 69, 70, 70, 71, 71, 48
		};

		MeshId& sphereMeshId = this->staticMeshes[static_cast<unsigned int>(PrimitiveType::WireSphere)];
		sphereMeshId = meshManager->CreateMesh();

		IndexedVertexData data;
		data.vertexFormat = vertexFormatPos;
		data.primitiveMode = RenderPrimitiveMode::Lines;
		data.vertexData = sphereVertexData;
		data.vertexDataSize = sizeof(sphereVertexData);
		data.vertexCount = sphereVertices;
		data.indexData = sphereIndexData;
		data.indexDataSize = sizeof(sphereIndexData);
		data.indexCount = sizeof(sphereIndexData) / sizeof(sphereIndexData[0]);

		meshManager->UploadIndexed(sphereMeshId, data);
	}

	{
		float rectangleVertexData[] = {
			-0.5f, 0.5f, 0.0f,
			0.5f, 0.5f, 0.0f,
			0.5f, -0.5f, 0.0f,
			-0.5f, -0.5f, 0.0f
		};

		unsigned short rectangleIndexData[] = { 0, 1, 1, 2, 2, 3, 3, 0 };

		MeshId& rectangleMeshId = this->staticMeshes[static_cast<unsigned int>(PrimitiveType::Rectangle)];
		rectangleMeshId = meshManager->CreateMesh();

		IndexedVertexData data;
		data.vertexFormat = vertexFormatPos;
		data.primitiveMode = RenderPrimitiveMode::Lines;
		data.vertexData = rectangleVertexData;
		data.vertexDataSize = sizeof(rectangleVertexData);
		data.vertexCount = sizeof(rectangleVertexData) / (sizeof(rectangleVertexData[0]) * 3);
		data.indexData = rectangleIndexData;
		data.indexDataSize = sizeof(rectangleIndexData);
		data.indexCount = sizeof(rectangleIndexData) / sizeof(rectangleIndexData[0]);

		meshManager->UploadIndexed(rectangleMeshId, data);
	}
}

void DebugVectorRenderer::Deinitialize()
{
	allocator->Deallocate(primitives);
	primitives = nullptr;

	if (meshesInitialized)
	{
		for (unsigned int i = 0; i < 4; ++i)
			meshManager->RemoveMesh(staticMeshes[i]);

		meshesInitialized = false;
	}

	if (uniformBufferId != 0)
	{
		renderDevice->DestroyBuffers(1, &uniformBufferId);
		uniformBufferId = render::BufferId();
		bufferPrimitivesAllocated = 0;
	}
}

DebugVectorRenderer::DynamicMesh* DebugVectorRenderer::GetDynamicMesh(size_t byteSize)
{
	int bestFit = -1;
	size_t bestFitSize;
	int largestUnused = -1;
	size_t largestUnusedSize;

	for (unsigned int i = 0, count = dynamicMeshCount; i < count; ++i)
	{
		const DynamicMesh& mesh = dynamicMeshes[i];
		if (mesh.used == false)
		{
			if (largestUnused == -1 || largestUnusedSize < mesh.bufferSize)
			{
				largestUnused = i;
				largestUnusedSize = mesh.bufferSize;
			}

			if (mesh.bufferSize >= byteSize)
			{
				if (bestFit == -1 || bestFitSize > mesh.bufferSize)
				{
					bestFit = i;
					bestFitSize = mesh.bufferSize;
				}
			}
		}
	}

	if (bestFit != -1)
	{
		return &dynamicMeshes[bestFit];
	}
	else if (largestUnused != -1)
	{
		return &dynamicMeshes[largestUnused];
	}
	else
	{
		if (dynamicMeshCount == dynamicMeshAllocated)
		{
			unsigned int newAllocated = dynamicMeshAllocated == 0 ? 8 : dynamicMeshAllocated * 2;
			size_t newByteSize = newAllocated * sizeof(DynamicMesh);
			DynamicMesh* newData = static_cast<DynamicMesh*>(allocator->Allocate(newByteSize, "DebugVectorRenderer.dynamicMeshes"));

			if (dynamicMeshCount > 0)
				std::memcpy(newData, dynamicMeshes, dynamicMeshCount * sizeof(DynamicMesh));

			dynamicMeshes = newData;
			dynamicMeshAllocated = newAllocated;
		}

		DynamicMesh* mesh = &dynamicMeshes[dynamicMeshCount];
		dynamicMeshCount += 1;

		mesh->meshId = meshManager->CreateMesh();
		mesh->bufferSize = 0;
		mesh->used = false;

		return mesh;
	}
}

void DebugVectorRenderer::DrawLineScreen(const Vec2f& start, const Vec2f& end, const Color& color)
{
	if (primitiveCount < primitiveAllocated)
	{
		Vec3f start3(start.x, start.y, 0.0f);
		Vec3f end3(end.x, end.y, 0.0f);
		float len = (end3 - start3).Magnitude();

		Primitive* prim = primitives + primitiveCount;
		prim->screenSpace = true;
		prim->type = PrimitiveType::Line;
		prim->transform = Mat4x4f::LookAt(start3, end3, Vec3f(0.0f, 0.0f, 1.0f)) * Mat4x4f::Scale(len);
		prim->color = color;
		prim->dynamicMeshIndex = -1;

		++primitiveCount;
	}
}

void DebugVectorRenderer::DrawLineChainScreen(size_t count, const Vec3f* points, const Color& color)
{
	if (primitiveCount < primitiveAllocated)
	{
		Primitive* prim = primitives + primitiveCount;
		prim->screenSpace = true;
		prim->type = PrimitiveType::LineChain;
		prim->transform = Mat4x4f();
		prim->color = color;

		size_t requiredBufferSize = count * sizeof(points[0]);

		// Find or create dynamic mesh to use
		DynamicMesh* mesh = GetDynamicMesh(requiredBufferSize);

		VertexAttribute vertexAttributes[] = { VertexAttribute::pos3 };
		VertexFormat vertexFormatPos(vertexAttributes, sizeof(vertexAttributes) / sizeof(vertexAttributes[0]));

		VertexData data;
		data.vertexFormat = vertexFormatPos;
		data.primitiveMode = RenderPrimitiveMode::LineStrip;
		data.vertexData = reinterpret_cast<const float*>(points);
		data.vertexDataSize = static_cast<unsigned int>(requiredBufferSize);
		data.vertexCount = static_cast<unsigned int>(count);

		meshManager->Upload(mesh->meshId, data);

		if (requiredBufferSize > mesh->bufferSize)
			mesh->bufferSize = requiredBufferSize;

		mesh->used = true;

		prim->dynamicMeshIndex = static_cast<int>(mesh - dynamicMeshes);

		++primitiveCount;
	}
}

void DebugVectorRenderer::DrawLine(const Vec3f& start, const Vec3f& end, const Color& color)
{
	if (primitiveCount < primitiveAllocated)
	{
		Vec3f dir = end - start;
		float len = dir.Magnitude();
		Vec3f up = std::abs(dir.y / len) < 0.9f ? Vec3f(0.0f, 1.0f, 0.0f) : Vec3f(1.0f, 0.0f, 0.0f);

		Primitive* prim = primitives + primitiveCount;
		prim->screenSpace = false;
		prim->type = PrimitiveType::Line;
		prim->transform = Mat4x4f::LookAt(start, end, up) * Mat4x4f::Scale(len);
		prim->color = color;
		prim->dynamicMeshIndex = -1;

		++primitiveCount;
	}
}

void DebugVectorRenderer::DrawWireCube(const Mat4x4f& transform, const Color& color)
{
	if (primitiveCount < primitiveAllocated)
	{
		Primitive* prim = primitives + primitiveCount;
		prim->screenSpace = false;
		prim->type = PrimitiveType::WireCube;
		prim->transform = transform;
		prim->color = color;
		prim->dynamicMeshIndex = -1;

		++primitiveCount;
	}
}

void DebugVectorRenderer::DrawWireSphere(const Vec3f& position, float radius, const Color& color)
{
	if (primitiveCount < primitiveAllocated)
	{
		Primitive* prim = primitives + primitiveCount;
		prim->screenSpace = false;
		prim->type = PrimitiveType::WireSphere;
		prim->transform = Mat4x4f::Translate(position) * Mat4x4f::Scale(radius);
		prim->color = color;
		prim->dynamicMeshIndex = -1;

		++primitiveCount;
	}
}

void DebugVectorRenderer::DrawRectangleScreen(const Rectanglef& rectangle, const Color& color)
{
	if (primitiveCount < primitiveAllocated)
	{
		Vec2f center = rectangle.position + rectangle.size * 0.5f;
		Vec3f center3(center.x, center.y, 0.0f);
		Vec3f scale(rectangle.size.x, rectangle.size.y, 1.0f);

		Primitive* prim = primitives + primitiveCount;
		prim->screenSpace = true;
		prim->type = PrimitiveType::Rectangle;
		prim->transform = Mat4x4f::Translate(center3) * Mat4x4f::Scale(scale);
		prim->color = color;
		prim->dynamicMeshIndex = -1;

		++primitiveCount;
	}
}

void DebugVectorRenderer::DrawWireFrustum(const Mat4x4f& transform, const ProjectionParameters& projection, const Color& color)
{
	FrustumPoints frustum;
	frustum.Update(projection, transform);

	this->DrawLine(frustum.points[0], frustum.points[1], color);
	this->DrawLine(frustum.points[0], frustum.points[2], color);
	this->DrawLine(frustum.points[1], frustum.points[3], color);
	this->DrawLine(frustum.points[2], frustum.points[3], color);

	this->DrawLine(frustum.points[0], frustum.points[4], color);
	this->DrawLine(frustum.points[1], frustum.points[5], color);
	this->DrawLine(frustum.points[2], frustum.points[6], color);
	this->DrawLine(frustum.points[3], frustum.points[7], color);

	this->DrawLine(frustum.points[4], frustum.points[5], color);
	this->DrawLine(frustum.points[4], frustum.points[6], color);
	this->DrawLine(frustum.points[5], frustum.points[7], color);
	this->DrawLine(frustum.points[6], frustum.points[7], color);
}

void DebugVectorRenderer::Render(kokko::render::CommandEncoder* encoder, World* world, const ViewRectangle& viewport, const Optional<CameraParameters>& editorCamera)
{
	KOKKO_PROFILE_FUNCTION();

	if (primitiveCount > 0)
	{
		Mat4x4f viewProj;
		Mat4x4f screenProj = Mat4x4f::ScreenSpaceProjection(viewport.size);

		if (editorCamera.HasValue())
		{
			const CameraParameters& cameraParams = editorCamera.GetValue();

			bool reverseDepth = false;
			Mat4x4f proj = cameraParams.projection.GetProjectionMatrix(reverseDepth);
			Mat4x4f view = cameraParams.transform.inverse;
			viewProj = proj * view;
		}
		else
		{
			Scene* scene = world->GetScene();
			kokko::CameraSystem* cameraSystem = world->GetCameraSystem();

			Entity cameraEntity = cameraSystem->GetActiveCamera();
			SceneObjectId cameraSceneObject = scene->Lookup(cameraEntity);
			const Mat4x4f& cameraTransform = scene->GetWorldTransform(cameraSceneObject);
			Optional<Mat4x4f> viewOpt = cameraTransform.GetInverse();
			Mat4x4f view;

			if (viewOpt.HasValue())
				view = viewOpt.GetValue();
			else
				KK_LOG_ERROR("DebugVectorRenderer: camera's transform couldn't be inverted");

			kokko::CameraId cameraId = cameraSystem->Lookup(cameraEntity);
			ProjectionParameters projectionParams = cameraSystem->GetProjection(cameraId);
			projectionParams.SetAspectRatio(viewport.size.x, viewport.size.y);

			bool reverseDepth = false;
			Mat4x4f proj = projectionParams.GetProjectionMatrix(reverseDepth);
			viewProj = proj * view;
		}

		DebugVectorBlock objectUniforms;

		{
			auto scope = renderDevice->CreateDebugScope(0, kokko::ConstStringView("DebugVec_Upload"));

			if (bufferPrimitivesAllocated != 0 && primitiveCount > bufferPrimitivesAllocated)
			{
				renderDevice->DestroyBuffers(1, &uniformBufferId);
				uniformBufferId = kokko::render::BufferId();
				bufferPrimitivesAllocated = 0;
			}

			if (uniformBufferId == 0)
			{
				bufferPrimitivesAllocated = primitiveCount;
				uint32_t size = bufferAlignedSize * bufferPrimitivesAllocated;

				renderDevice->CreateBuffers(1, &uniformBufferId);
				renderDevice->SetBufferStorage(uniformBufferId, size, nullptr, BufferStorageFlags::Dynamic);
			}

			for (unsigned int i = 0; i < primitiveCount; ++i)
			{
				const Primitive& primitive = primitives[i];

				// Use view or screen based transform depending on whether this is a screen-space primitive
				Mat4x4f mvp = (primitive.screenSpace ? screenProj : viewProj) * primitive.transform;

				// Update uniform buffer

				objectUniforms.transform = mvp;
				Vec4f colorVec4(primitive.color.r, primitive.color.g, primitive.color.b, primitive.color.a);
				objectUniforms.color = colorVec4;

				uint32_t offset = bufferAlignedSize * i;
				renderDevice->SetBufferSubData(uniformBufferId, offset, sizeof(DebugVectorBlock), &objectUniforms);
			}
		}

		{
			auto scope = encoder->CreateDebugScope(0, kokko::ConstStringView("DebugVec_Render"));

			const ShaderData& shader = shaderManager->GetShaderData(shaderId);

			encoder->DepthTestDisable();
			encoder->BlendingDisable();
			encoder->SetViewport(viewport.position.x, viewport.position.y, viewport.size.x, viewport.size.y);

			encoder->UseShaderProgram(shader.driverId);

			// FIXME: A single buffer can no longer be shared between draw calls

			for (unsigned int i = 0; i < primitiveCount; ++i)
			{
				const Primitive& primitive = primitives[i];

				uint32_t offset = bufferAlignedSize * i;
				encoder->BindBufferRange(RenderBufferTarget::UniformBuffer, UniformBlockBinding::Object,
					uniformBufferId, offset, sizeof(DebugVectorBlock));

				// Draw

				MeshId meshId;

				if (primitive.dynamicMeshIndex >= 0)
					meshId = this->dynamicMeshes[primitive.dynamicMeshIndex].meshId;
				else
					meshId = this->staticMeshes[static_cast<unsigned int>(primitive.type)];

				const MeshDrawData* draw = meshManager->GetDrawData(meshId);
				encoder->BindVertexArray(draw->vertexArrayObject);

				if (draw->indexType != RenderIndexType::None)
					encoder->DrawIndexed(draw->primitiveMode, draw->indexType, draw->count, 0, 0);
				else
					encoder->Draw(draw->primitiveMode, 0, draw->count);
			}
		}

		// Clear primitive count
		primitiveCount = 0;

		// Free dynamic meshes for use
		for (unsigned int i = 0, count = dynamicMeshCount; i < count; ++i)
			dynamicMeshes[i].used = false;
	}
}

} // namespace kokko
