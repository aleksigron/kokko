#include "Debug/DebugVectorRenderer.hpp"

#include <cstring>
#include <cmath>

#include "Engine/Engine.hpp"

#include "Math/Math.hpp"

#include "Rendering/Camera.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/Shader.hpp"
#include "Rendering/UniformBufferData.hpp"

#include "Resources/ResourceManager.hpp"
#include "Resources/MeshManager.hpp"

#include "Scene/Scene.hpp"
#include "Scene/SceneManager.hpp"

#include "System/Window.hpp"
#include "System/IncludeOpenGL.hpp"

DebugVectorRenderer::DebugVectorRenderer(Allocator* allocator, RenderDevice* renderDevice) :
	allocator(allocator),
	renderDevice(renderDevice),
	meshesInitialized(false),
	objectUniformBufferId(0)
{
	primitiveCount = 0;
	primitiveAllocated = 1024;

	std::size_t primitivesSize = sizeof(Primitive) * primitiveAllocated;
	primitives = static_cast<Primitive*>(allocator->Allocate(primitivesSize));
}

DebugVectorRenderer::~DebugVectorRenderer()
{
	allocator->Deallocate(primitives);

	if (meshesInitialized)
	{
		Engine* engine = Engine::GetInstance();
		MeshManager* meshManager = engine->GetMeshManager();
		for (unsigned int i = 0; i < 4; ++i)
			meshManager->RemoveMesh(meshIds[i]);
	}

	if (objectUniformBufferId != 0)
	{
		renderDevice->DestroyBuffers(1, &objectUniformBufferId);
	}
}

void DebugVectorRenderer::CreateMeshes()
{
	Engine* engine = Engine::GetInstance();
	MeshManager* meshManager = engine->GetMeshManager();

	{
		Vertex3f lineVertexData[] = {
			Vertex3f{ Vec3f(0.0f, 0.0f, 0.0f) },
			Vertex3f{ Vec3f(0.0f, 0.0f, -1.0f) }
		};

		unsigned short lineIndexData[] = { 0, 1 };

		MeshId& lineMeshId = this->meshIds[static_cast<unsigned int>(PrimitiveType::Line)];
		lineMeshId = meshManager->CreateMesh();

		IndexedVertexData<Vertex3f, unsigned short> data;
		data.primitiveMode = MeshPrimitiveMode::Lines;
		data.vertData = lineVertexData;
		data.vertCount = sizeof(lineVertexData) / sizeof(Vertex3f);
		data.idxData = lineIndexData;
		data.idxCount = sizeof(lineIndexData) / sizeof(unsigned short);

		meshManager->Upload_3f(lineMeshId, data, RenderData::BufferUsage::StaticDraw);
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

		MeshId& cubeMeshId = this->meshIds[static_cast<unsigned int>(PrimitiveType::WireCube)];
		cubeMeshId = meshManager->CreateMesh();

		IndexedVertexData<Vertex3f, unsigned short> data;
		data.primitiveMode = MeshPrimitiveMode::Lines;
		data.vertData = cubeVertexData;
		data.vertCount = sizeof(cubeVertexData) / sizeof(Vertex3f);
		data.idxData = cubeIndexData;
		data.idxCount = sizeof(cubeIndexData) / sizeof(unsigned short);

		meshManager->Upload_3f(cubeMeshId, data, RenderData::BufferUsage::StaticDraw);
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

		MeshId& sphereMeshId = this->meshIds[static_cast<unsigned int>(PrimitiveType::WireSphere)];
		sphereMeshId = meshManager->CreateMesh();

		IndexedVertexData<Vertex3f, unsigned short> data;
		data.primitiveMode = MeshPrimitiveMode::Lines;
		data.vertData = sphereVertexData;
		data.vertCount = sizeof(sphereVertexData) / sizeof(Vertex3f);
		data.idxData = sphereIndexData;
		data.idxCount = sizeof(sphereIndexData) / sizeof(unsigned short);

		meshManager->Upload_3f(sphereMeshId, data, RenderData::BufferUsage::StaticDraw);
	}

	{
		Vertex3f rectangleVertexData[] = {
			Vertex3f{ Vec3f(-0.5f, 0.5f, 0.0f) },
			Vertex3f{ Vec3f(0.5f, 0.5f, 0.0f) },
			Vertex3f{ Vec3f(0.5f, -0.5f, 0.0f) },
			Vertex3f{ Vec3f(-0.5f, -0.5f, 0.0f) }
		};

		unsigned short rectangleIndexData[] = { 0, 1, 2, 3 };

		MeshId& rectangleMeshId = this->meshIds[static_cast<unsigned int>(PrimitiveType::Rectangle)];
		rectangleMeshId = meshManager->CreateMesh();

		IndexedVertexData<Vertex3f, unsigned short> data;
		data.primitiveMode = MeshPrimitiveMode::Lines;
		data.vertData = rectangleVertexData;
		data.vertCount = sizeof(rectangleVertexData) / sizeof(Vertex3f);
		data.idxData = rectangleIndexData;
		data.idxCount = sizeof(rectangleIndexData) / sizeof(unsigned short);

		meshManager->Upload_3f(rectangleMeshId, data, RenderData::BufferUsage::StaticDraw);
	}

	meshesInitialized = true;
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
		Engine* engine = Engine::GetInstance();
		MeshManager* meshManager = engine->GetMeshManager();
		ResourceManager* rm = engine->GetResourceManager();
		Shader* shader = rm->GetShader("res/shaders/debug/debug_vector.shader.json");

		if (shader == nullptr)
			return;

		if (meshesInitialized == false)
			this->CreateMeshes();

		if (objectUniformBufferId == 0)
		{
			renderDevice->CreateBuffers(1, &objectUniformBufferId);
			renderDevice->BindBuffer(GL_UNIFORM_BUFFER, objectUniformBufferId);
			renderDevice->SetBufferData(GL_UNIFORM_BUFFER, UniformBuffer::TransformBlock::BufferSize, nullptr, RenderData::BufferUsage::DynamicDraw);
		}

		SceneManager* sm = engine->GetSceneManager();
		Scene* scene = sm->GetScene(sm->GetPrimarySceneId());
		SceneObjectId cameraSceneObject = scene->Lookup(camera->GetEntity());
		const Mat4x4f& cameraTransform = scene->GetWorldTransform(cameraSceneObject);

		Mat4x4f proj = camera->parameters.GetProjectionMatrix();
		Mat4x4f view = Camera::GetViewMatrix(cameraTransform);
		Mat4x4f viewProj = proj * view;
		Mat4x4f screenProj = engine->GetMainWindow()->GetScreenSpaceProjectionMatrix();

		unsigned char objectUboBuffer[UniformBuffer::TransformBlock::BufferSize];

		int colorUniformLocation = -1;
		for (unsigned int i = 0; i < shader->materialUniformCount; ++i)
		{
			if (shader->materialUniforms[i].type == ShaderUniformType::Vec4)
			{
				colorUniformLocation = shader->materialUniforms[i].location;
			}
		}

		renderDevice->DepthTestDisable();
		renderDevice->BlendingDisable();

		// Use shader
		renderDevice->UseShaderProgram(shader->driverId);

		for (unsigned int i = 0; i < primitiveCount; ++i)
		{
			const Primitive& primitive = primitives[i];

			// Set color uniform
			renderDevice->SetUniformVec4f(colorUniformLocation, 1, primitive.color.ValuePointer());

			// Use view or screen based transform depending on whether this is a screen-space primitive
			Mat4x4f mvp = (primitive.screenSpace ? screenProj : viewProj) * primitive.transform;

			// Update object transform uniform buffer

			UniformBuffer::TransformBlock::MVP.Set(objectUboBuffer, mvp);
			UniformBuffer::TransformBlock::MV.Set(objectUboBuffer, view * primitive.transform);
			UniformBuffer::TransformBlock::M.Set(objectUboBuffer, primitive.transform);

			renderDevice->BindBuffer(GL_UNIFORM_BUFFER, objectUniformBufferId);
			renderDevice->SetBufferSubData(GL_UNIFORM_BUFFER, 0, UniformBuffer::TransformBlock::BufferSize, objectUboBuffer);

			// Bind uniform block to shader
			renderDevice->BindBufferBase(GL_UNIFORM_BUFFER, UniformBuffer::TransformBlock::BindingPoint, objectUniformBufferId);

			// Draw

			MeshId meshId = this->meshIds[static_cast<unsigned int>(primitive.type)];
			MeshDrawData* draw = meshManager->GetDrawData(meshId);
			renderDevice->BindVertexArray(draw->vertexArrayObject);
			renderDevice->DrawVertexArray(draw->primitiveMode, draw->indexCount, draw->indexElementType);
		}

		// Clear primitive count
		primitiveCount = 0;
	}
}

