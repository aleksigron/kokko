#include "DebugVectorRenderer.hpp"

#include <cstring>
#include <cmath>

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

#include "Math.hpp"
#include "App.hpp"
#include "Camera.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"
#include "ResourceManager.hpp"

DebugVectorRenderer::DebugVectorRenderer()
{
	primitiveCount = 0;
	primitiveAllocated = 64;
	primitives = new Primitive[primitiveAllocated];

	this->CreateMeshes();
}

DebugVectorRenderer::~DebugVectorRenderer()
{
	delete[] primitives;
}

void DebugVectorRenderer::CreateMeshes()
{
	ResourceManager* rm = App::GetResourceManager();

	{
		Vertex3f lineVertexData[] = {
			Vertex3f{ Vec3f(0.0f, 0.0f, 0.0f) },
			Vertex3f{ Vec3f(0.0f, 0.0f, -1.0f) }
		};

		unsigned short lineIndexData[] = { 0, 1 };

		ObjectId& lineMeshId = this->meshIds[static_cast<unsigned int>(PrimitiveType::Line)];
		lineMeshId = rm->meshes.Add();
		Mesh& lineMesh = rm->meshes.Get(lineMeshId);

		BufferRef<Vertex3f> vertices;
		vertices.data = lineVertexData;
		vertices.count = sizeof(lineVertexData) / sizeof(Vertex3f);

		BufferRef<unsigned short> indices;
		indices.data = lineIndexData;
		indices.count = sizeof(lineIndexData) / sizeof(unsigned short);

		lineMesh.primitiveMode = GL_LINES;
		lineMesh.Upload_3f(vertices, indices);
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

		ObjectId& cubeMeshId = this->meshIds[static_cast<unsigned int>(PrimitiveType::Cube)];
		cubeMeshId = rm->meshes.Add();
		Mesh& cubeMesh = rm->meshes.Get(cubeMeshId);

		BufferRef<Vertex3f> vertices;
		vertices.data = cubeVertexData;
		vertices.count = sizeof(cubeVertexData) / sizeof(Vertex3f);

		BufferRef<unsigned short> indices;
		indices.data = cubeIndexData;
		indices.count = sizeof(cubeIndexData) / sizeof(unsigned short);

		cubeMesh.primitiveMode = GL_LINES;
		cubeMesh.Upload_3f(vertices, indices);
	}

	{
		static const int sphereVertices = 72;

		Vertex3f sphereVertexData[sphereVertices];

		for (int i = 0; i < 24; ++i)
		{
			float f = Mathf::Constant::Tau / 24 * i;
			sphereVertexData[0 + i] = Vertex3f{ Vec3f(std::sin(f), std::cos(f), 0.0f) };
		}

		for (int i = 0; i < 24; ++i)
		{
			float f = Mathf::Constant::Tau / 24 * i;
			sphereVertexData[24 + i] = Vertex3f{ Vec3f(0.0f, std::sin(f), std::cos(f)) };
		}

		for (int i = 0; i < 24; ++i)
		{
			float f = Mathf::Constant::Tau / 24 * i;
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

		ObjectId& sphereMeshId = this->meshIds[static_cast<unsigned int>(PrimitiveType::Sphere)];
		sphereMeshId = rm->meshes.Add();
		Mesh& sphereMesh = rm->meshes.Get(sphereMeshId);

		BufferRef<Vertex3f> vertices;
		vertices.data = sphereVertexData;
		vertices.count = sizeof(sphereVertexData) / sizeof(Vertex3f);

		BufferRef<unsigned short> indices;
		indices.data = sphereIndexData;
		indices.count = sizeof(sphereIndexData) / sizeof(unsigned short);

		sphereMesh.primitiveMode = GL_LINES;
		sphereMesh.Upload_3f(vertices, indices);
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
		prim->type = PrimitiveType::Line;
		prim->transform = Mat4x4f::LookAt(start, end, up) * Mat4x4f::Scale(len);
		prim->color = color;

		++primitiveCount;
	}
}

void DebugVectorRenderer::DrawCube(const Mat4x4f& transform, const Color& color)
{
	if (primitiveCount < primitiveAllocated)
	{
		Primitive* prim = primitives + primitiveCount;
		prim->type = PrimitiveType::Cube;
		prim->transform = transform;
		prim->color = color;

		++primitiveCount;
	}
}

void DebugVectorRenderer::DrawSphere(const Vec3f& position, float radius, const Color& color)
{
	if (primitiveCount < primitiveAllocated)
	{
		Primitive* prim = primitives + primitiveCount;
		prim->type = PrimitiveType::Sphere;
		prim->transform = Mat4x4f::Translate(position) * Mat4x4f::Scale(radius);
		prim->color = color;

		++primitiveCount;
	}
}

void DebugVectorRenderer::Render(const Camera& camera)
{
	if (primitiveCount > 0)
	{
		Mat4x4f viewProjection = camera.GetProjectionMatrix() * camera.GetViewMatrix();

		ResourceManager* rm = App::GetResourceManager();
		Shader* shader = rm->GetShader("res/shaders/debug_vector.shader.json");

		int colorUniformLocation = -1;
		for (unsigned int i = 0; i < shader->materialUniformCount; ++i)
		{
			if (shader->materialUniforms[i].type == ShaderUniformType::Vec4)
			{
				colorUniformLocation = shader->materialUniforms[i].location;
			}
		}

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		// Use shader
		glUseProgram(shader->driverId);

		for (unsigned int i = 0; i < primitiveCount; ++i)
		{
			Primitive* primitive = primitives + i;
			Mat4x4f mvp = viewProjection * primitive->transform;

			ObjectId meshId = this->meshIds[static_cast<unsigned int>(primitive->type)];
			const Mesh& mesh = rm->meshes.Get(meshId);

			// Set color uniform
			glUniform4fv(colorUniformLocation, 1, primitive->color.ValuePointer());

			// Bind vertex array object
			glBindVertexArray(mesh.vertexArrayObject);

			// Set transform matrix uniform
			glUniformMatrix4fv(shader->uniformMatMVP, 1, GL_FALSE, mvp.ValuePointer());

			// Draw
			glDrawElements(mesh.primitiveMode, mesh.indexCount, mesh.indexElementType, nullptr);
		}

		// Clear primitive count
		primitiveCount = 0;
	}
}

