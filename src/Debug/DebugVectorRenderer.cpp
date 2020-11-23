#include "Debug/DebugVectorRenderer.hpp"

#include <cstring>
#include <cmath>

#include "Engine/Engine.hpp"

#include "Math/Math.hpp"

#include "Rendering/Camera.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/UniformBufferData.hpp"

#include "Resources/MeshManager.hpp"
#include "Resources/ShaderManager.hpp"

#include "Scene/Scene.hpp"
#include "Scene/SceneManager.hpp"

#include "System/Window.hpp"

struct MaterialBlock
{
	static const std::size_t BufferSize = 16;

	static UniformBuffer::ScalarUniform<Vec4f, 0> color;
};

DebugVectorRenderer::DebugVectorRenderer(
	Allocator* allocator,
	RenderDevice* renderDevice) :
	allocator(allocator),
	renderDevice(renderDevice),
	shaderManager(nullptr),
	meshManager(nullptr),
	dynamicMeshes(nullptr),
	dynamicMeshCount(0),
	dynamicMeshAllocated(0),
	meshesInitialized(false),
	shaderId(ShaderId{0}),
	buffersInitialized(false)
{
	primitiveCount = 0;
	primitiveAllocated = 1024;

	std::size_t primitivesSize = sizeof(Primitive) * primitiveAllocated;
	primitives = static_cast<Primitive*>(allocator->Allocate(primitivesSize));
}

DebugVectorRenderer::~DebugVectorRenderer()
{
}

void DebugVectorRenderer::Initialize(MeshManager* meshManager, ShaderManager* shaderManager)
{
	// Initialize shaders

	this->shaderManager = shaderManager;

	const char* shaderPath = "res/shaders/debug/debug_vector.shader.json";
	shaderId = shaderManager->GetIdByPath(StringRef(shaderPath));

	// Initialize meshes

	this->meshManager = meshManager;

	{
		Vertex3f lineVertexData[] = {
			Vertex3f{ Vec3f(0.0f, 0.0f, 0.0f) },
			Vertex3f{ Vec3f(0.0f, 0.0f, -1.0f) }
		};

		unsigned short lineIndexData[] = { 0, 1 };

		MeshId& lineMeshId = this->staticMeshes[static_cast<unsigned int>(PrimitiveType::Line)];
		lineMeshId = meshManager->CreateMesh();

		IndexedVertexData<Vertex3f, unsigned short> data;
		data.primitiveMode = RenderPrimitiveMode::Lines;
		data.vertData = lineVertexData;
		data.vertCount = sizeof(lineVertexData) / sizeof(Vertex3f);
		data.idxData = lineIndexData;
		data.idxCount = sizeof(lineIndexData) / sizeof(unsigned short);

		meshManager->UploadIndexed_3f(lineMeshId, data, RenderBufferUsage::StaticDraw);
	}

	{
		Vertex3f cubeVertexData[] = {
			Vertex3f{ Vec3f(-0.5f, -0.5f, -0.5f) },
			Vertex3f{ Vec3f(0.5f, -0.5f, -0.5f) },
			Vertex3f{ Vec3f(-0.5f, -0.5f, 0.5f) },
			Vertex3f{ Vec3f(0.5f, -0.5f, 0.5f) },
			Vertex3f{ Vec3f(-0.5f, 0.5f, -0.5f) },
			Vertex3f{ Vec3f(0.5f, 0.5f, -0.5f) },
			Vertex3f{ Vec3f(-0.5f, 0.5f, 0.5f) },
			Vertex3f{ Vec3f(0.5f, 0.5f, 0.5f) }
		};

		unsigned short cubeIndexData[] = {
			0, 1, 1, 3, 3, 2, 2, 0,
			0, 4, 1, 5, 2, 6, 3, 7,
			4, 5, 5, 7, 7, 6, 6, 4
		};

		MeshId& cubeMeshId = this->staticMeshes[static_cast<unsigned int>(PrimitiveType::WireCube)];
		cubeMeshId = meshManager->CreateMesh();

		IndexedVertexData<Vertex3f, unsigned short> data;
		data.primitiveMode = RenderPrimitiveMode::Lines;
		data.vertData = cubeVertexData;
		data.vertCount = sizeof(cubeVertexData) / sizeof(Vertex3f);
		data.idxData = cubeIndexData;
		data.idxCount = sizeof(cubeIndexData) / sizeof(unsigned short);

		meshManager->UploadIndexed_3f(cubeMeshId, data, RenderBufferUsage::StaticDraw);
	}

	{
		static const int sphereVertices = 72;

		Vertex3f sphereVertexData[sphereVertices];

		for (int i = 0; i < 24; ++i)
		{
			float f = Math::Const::Tau / 24 * i;
			sphereVertexData[0 + i] = Vertex3f{ Vec3f(std::sin(f), std::cos(f), 0.0f) };
		}

		for (int i = 0; i < 24; ++i)
		{
			float f = Math::Const::Tau / 24 * i;
			sphereVertexData[24 + i] = Vertex3f{ Vec3f(0.0f, std::sin(f), std::cos(f)) };
		}

		for (int i = 0; i < 24; ++i)
		{
			float f = Math::Const::Tau / 24 * i;
			sphereVertexData[48 + i] = Vertex3f{ Vec3f(std::cos(f), 0.0f, std::sin(f)) };
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

		IndexedVertexData<Vertex3f, unsigned short> data;
		data.primitiveMode = RenderPrimitiveMode::Lines;
		data.vertData = sphereVertexData;
		data.vertCount = sizeof(sphereVertexData) / sizeof(Vertex3f);
		data.idxData = sphereIndexData;
		data.idxCount = sizeof(sphereIndexData) / sizeof(unsigned short);

		meshManager->UploadIndexed_3f(sphereMeshId, data, RenderBufferUsage::StaticDraw);
	}

	{
		Vertex3f rectangleVertexData[] = {
			Vertex3f{ Vec3f(-0.5f, 0.5f, 0.0f) },
			Vertex3f{ Vec3f(0.5f, 0.5f, 0.0f) },
			Vertex3f{ Vec3f(0.5f, -0.5f, 0.0f) },
			Vertex3f{ Vec3f(-0.5f, -0.5f, 0.0f) }
		};

		unsigned short rectangleIndexData[] = { 0, 1, 2, 3 };

		MeshId& rectangleMeshId = this->staticMeshes[static_cast<unsigned int>(PrimitiveType::Rectangle)];
		rectangleMeshId = meshManager->CreateMesh();

		IndexedVertexData<Vertex3f, unsigned short> data;
		data.primitiveMode = RenderPrimitiveMode::Lines;
		data.vertData = rectangleVertexData;
		data.vertCount = sizeof(rectangleVertexData) / sizeof(Vertex3f);
		data.idxData = rectangleIndexData;
		data.idxCount = sizeof(rectangleIndexData) / sizeof(unsigned short);

		meshManager->UploadIndexed_3f(rectangleMeshId, data, RenderBufferUsage::StaticDraw);
	}
}

void DebugVectorRenderer::Deinitialize()
{
	allocator->Deallocate(primitives);
	primitives = nullptr;

	if (meshesInitialized)
	{
		Engine* engine = Engine::GetInstance();
		MeshManager* meshManager = engine->GetMeshManager();
		for (unsigned int i = 0; i < 4; ++i)
			meshManager->RemoveMesh(staticMeshes[i]);

		meshesInitialized = false;
	}

	if (buffersInitialized)
	{
		renderDevice->DestroyBuffers(2, uniformBufferIds);
		buffersInitialized = false;
	}
}

void DebugVectorRenderer::CreateMeshes()
{
	Engine* engine = Engine::GetInstance();

	meshesInitialized = true;
}

DebugVectorRenderer::DynamicMesh* DebugVectorRenderer::GetDynamicMesh(unsigned int byteSize)
{
	int bestFit = -1;
	unsigned int bestFitSize;
	int largestUnused = -1;
	unsigned int largestUnusedSize;

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
			DynamicMesh* newData = static_cast<DynamicMesh*>(allocator->Allocate(newByteSize));

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

void DebugVectorRenderer::DrawLineChainScreen(unsigned int count, const Vec3f* points, const Color& color)
{
	if (primitiveCount < primitiveAllocated)
	{
		Primitive* prim = primitives + primitiveCount;
		prim->screenSpace = true;
		prim->type = PrimitiveType::LineChain;
		prim->transform = Mat4x4f();
		prim->color = color;

		unsigned int requiredBufferSize = count * sizeof(points[0]);

		// Find or create dynamic mesh to use
		DynamicMesh* mesh = GetDynamicMesh(requiredBufferSize);

		VertexData<Vertex3f> vertData;
		vertData.primitiveMode = RenderPrimitiveMode::LineStrip;
		vertData.vertData = reinterpret_cast<const Vertex3f*>(points);
		vertData.vertCount = count;

		meshManager->Upload_3f(mesh->meshId, vertData, RenderBufferUsage::DynamicDraw);

		if (requiredBufferSize > mesh->bufferSize)
			mesh->bufferSize = requiredBufferSize;

		mesh->used = true;

		prim->dynamicMeshIndex = mesh - dynamicMeshes;

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

void DebugVectorRenderer::Render(Camera* camera)
{
	if (primitiveCount > 0)
	{
		const ShaderData& shader = shaderManager->GetShaderData(shaderId);

		if (meshesInitialized == false)
			this->CreateMeshes();

		if (buffersInitialized == false)
		{
			RenderBufferUsage usage = RenderBufferUsage::DynamicDraw;

			renderDevice->CreateBuffers(2, uniformBufferIds);

			renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, uniformBufferIds[ObjectBuffer]);
			renderDevice->SetBufferData(RenderBufferTarget::UniformBuffer, UniformBuffer::TransformBlock::BufferSize, nullptr, usage);

			renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, uniformBufferIds[MaterialBuffer]);
			renderDevice->SetBufferData(RenderBufferTarget::UniformBuffer, MaterialBlock::BufferSize, nullptr, usage);

			buffersInitialized = true;
		}

		Engine* engine = Engine::GetInstance();
		SceneManager* sm = engine->GetSceneManager();
		Scene* scene = sm->GetScene(sm->GetPrimarySceneId());
		SceneObjectId cameraSceneObject = scene->Lookup(camera->GetEntity());
		const Mat4x4f& cameraTransform = scene->GetWorldTransform(cameraSceneObject);

		Mat4x4f proj = camera->parameters.GetProjectionMatrix();
		Mat4x4f view = Camera::GetViewMatrix(cameraTransform);
		Mat4x4f viewProj = proj * view;
		Mat4x4f screenProj = engine->GetMainWindow()->GetScreenSpaceProjectionMatrix();

		unsigned char objectUboBuffer[UniformBuffer::TransformBlock::BufferSize];
		unsigned char materialUboBuffer[MaterialBlock::BufferSize];

		renderDevice->DepthTestDisable();
		renderDevice->BlendingDisable();

		renderDevice->UseShaderProgram(shader.driverId);

		for (unsigned int i = 0; i < primitiveCount; ++i)
		{
			const Primitive& primitive = primitives[i];

			// Use view or screen based transform depending on whether this is a screen-space primitive
			Mat4x4f mvp = (primitive.screenSpace ? screenProj : viewProj) * primitive.transform;

			// Update object transform uniform buffer

			UniformBuffer::TransformBlock::MVP.Set(objectUboBuffer, mvp);
			UniformBuffer::TransformBlock::MV.Set(objectUboBuffer, view * primitive.transform);
			UniformBuffer::TransformBlock::M.Set(objectUboBuffer, primitive.transform);

			unsigned int objectBufferId = uniformBufferIds[ObjectBuffer];
			renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, objectBufferId);
			renderDevice->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, UniformBuffer::TransformBlock::BufferSize, objectUboBuffer);

			renderDevice->BindBufferBase(RenderBufferTarget::UniformBuffer, UniformBuffer::TransformBlock::BindingPoint, objectBufferId);

			// Update color

			Vec4f colorVec4(primitive.color.r, primitive.color.g, primitive.color.b, primitive.color.a);
			MaterialBlock::color.Set(materialUboBuffer, colorVec4);

			unsigned int materialBufferId = uniformBufferIds[MaterialBuffer];
			renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, materialBufferId);
			renderDevice->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, MaterialBlock::BufferSize, materialUboBuffer);

			renderDevice->BindBufferBase(RenderBufferTarget::UniformBuffer, UniformBuffer::MaterialBlock::BindingPoint, materialBufferId);

			// Draw

			MeshId meshId;

			if (primitive.dynamicMeshIndex >= 0)
				meshId = this->dynamicMeshes[primitive.dynamicMeshIndex].meshId;
			else
				meshId = this->staticMeshes[static_cast<unsigned int>(primitive.type)];

			MeshDrawData* draw = meshManager->GetDrawData(meshId);
			renderDevice->BindVertexArray(draw->vertexArrayObject);

			if (draw->indexType != RenderIndexType::None)
				renderDevice->DrawIndexed(draw->primitiveMode, draw->count, draw->indexType);
			else
				renderDevice->Draw(draw->primitiveMode, 0, draw->count);

		// Clear primitive count
		}
		primitiveCount = 0;

		// Free dynamic meshes for use
		for (unsigned int i = 0, count = dynamicMeshCount; i < count; ++i)
			dynamicMeshes[i].used = false;
	}
}

