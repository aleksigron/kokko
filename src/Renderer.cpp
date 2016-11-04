#include "Renderer.hpp"

#include <cassert>
#include <cstring>

#include <cstdio>

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

#include "Engine.hpp"
#include "App.hpp"
#include "Window.hpp"

#include "ResourceManager.hpp"
#include "Material.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Scene.hpp"
#include "Mesh.hpp"

#include "Camera.hpp"
#include "ViewFrustum.hpp"
#include "BoundingBox.hpp"
#include "FrustumCulling.hpp"
#include "RenderOrder.hpp"

#include "Sort.hpp"

static bool SortDrawCallsPredicate(const DrawCall& l, const DrawCall& r)
{
	return l.orderKey < r.orderKey;
}

Renderer::Renderer() :
	indexList(nullptr),
	freeListFirst(0),
	objects(nullptr),
	objectCount(0),
	allocatedCount(0),
	boundingBoxes(nullptr),
	cullingState(nullptr)
{
	this->InitializeRenderOrder();
}

Renderer::~Renderer()
{
	delete[] cullingState;
	delete[] boundingBoxes;
	delete[] objects;
	delete[] indexList;
}

void Renderer::InitializeRenderOrder()
{
	RenderOrderConfiguration& conf = renderOrderConfiguration;

	conf.viewportIndex.SetDefinition(4, sizeof(uint64_t) * 8);
	conf.viewportLayer.SetDefinition(4, conf.viewportIndex.shift);
	conf.transparencyType.SetDefinition(5, conf.viewportLayer.shift);
	conf.command.SetDefinition(1, conf.transparencyType.shift);

	conf.transparentDepth.SetDefinition(24, conf.command.shift);
	conf.transparentMaterialId.SetDefinition(16, conf.transparentDepth.shift);

	conf.opaqueDepth.SetDefinition(8, conf.command.shift);
	conf.opaqueMaterialId.SetDefinition(16, conf.opaqueDepth.shift);

	conf.commandType.SetDefinition(8, conf.command.shift);
	conf.commandData.SetDefinition(32, conf.commandType.shift);
}

void Renderer::PreTransformUpdate()
{
	Engine* engine = Engine::GetInstance();
	Scene* scene = engine->GetScene();

	Mat4x4f cameraTransform = scene->GetLocalTransform(this->activeCamera->GetSceneObjectId());
	Vec3f cameraPosition = (cameraTransform * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();

	// Update skybox transform
	scene->skybox.UpdateTransform(cameraPosition);
}

void Renderer::Render(Scene* scene)
{
	Engine* engine = Engine::GetInstance();
	ResourceManager* res = engine->GetResourceManager();

	Camera* cam = this->activeCamera;
	Mat4x4f cameraTransform = scene->GetLocalTransform(cam->GetSceneObjectId());

	// Update view frustum
	ViewFrustum frustum;
	frustum.UpdateFrustum(*cam, cameraTransform);

	this->UpdateBoundingBoxes(scene);

	// Do frustum culling
	FrustumCulling::CullAABB(&frustum, objectCount, this->boundingBoxes, this->cullingState);

	this->CreateDrawCalls(scene);

	// Sort draw calls based on order key
	ShellSortPred(drawCalls.GetData(), drawCalls.GetCount(), SortDrawCallsPredicate);

	// Get the background color for view
	Color clearCol = scene->backgroundColor;
	glClearColor(clearCol.r, clearCol.r, clearCol.r, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glDepthFunc(GL_LESS);

	glDisable(GL_BLEND);

	Mat4x4f viewMatrix = cam->GetViewMatrix();
	Mat4x4f projectionMatrix = cam->GetProjectionMatrix();
	Mat4x4f viewProjection = projectionMatrix * viewMatrix;

	for (unsigned index = 0, drawCallCount = drawCalls.GetCount(); index < drawCallCount; ++index)
	{
		const DrawCall& drawCall = drawCalls[index];

		RenderObject& obj = this->objects[drawCall.renderObjectIndex];

		Mesh& mesh = res->meshes.Get(obj.meshId);
		Material& material = res->GetMaterial(obj.materialId);
		Shader* shader = res->GetShader(material.shaderId);

		glUseProgram(shader->driverId);

		unsigned int usedTextures = 0;

		// Bind each material uniform with a value
		for (unsigned uIndex = 0; uIndex < material.uniformCount; ++uIndex)
		{
			ShaderMaterialUniform& u = material.uniforms[uIndex];

			unsigned char* d = material.uniformData + u.dataOffset;

			switch (u.type)
			{
				case ShaderUniformType::Mat4x4:
					glUniformMatrix4fv(u.location, 1, GL_FALSE, reinterpret_cast<float*>(d));
					break;

				case ShaderUniformType::Vec4:
					glUniform4fv(u.location, 1, reinterpret_cast<float*>(d));
					break;

				case ShaderUniformType::Vec3:
					glUniform3fv(u.location, 1, reinterpret_cast<float*>(d));
					break;

				case ShaderUniformType::Vec2:
					glUniform2fv(u.location, 1, reinterpret_cast<float*>(d));
					break;

				case ShaderUniformType::Float:
					glUniform1f(u.location, *reinterpret_cast<float*>(d));
					break;

				case ShaderUniformType::Int:
					glUniform1i(u.location, *reinterpret_cast<int*>(d));
					break;

				case ShaderUniformType::Tex2D:
				case ShaderUniformType::TexCube:
				{
					uint32_t textureHash = *reinterpret_cast<uint32_t*>(d);
					Texture* texture = res->GetTexture(textureHash);

					glActiveTexture(GL_TEXTURE0 + usedTextures);
					glBindTexture(texture->targetType, texture->driverId);
					glUniform1i(u.location, usedTextures);

					++usedTextures;
				}
					break;
			}
		}

		Mat4x4f modelMatrix = scene->GetWorldTransform(obj.sceneObjectId);

		if (shader->uniformMatMVP >= 0)
		{
			Mat4x4f mvp = viewProjection * modelMatrix;
			glUniformMatrix4fv(shader->uniformMatMVP, 1, GL_FALSE, mvp.ValuePointer());
		}

		if (shader->uniformMatMV >= 0)
		{
			Mat4x4f mv = viewMatrix * modelMatrix;
			glUniformMatrix4fv(shader->uniformMatMV, 1, GL_FALSE, mv.ValuePointer());
		}

		if (shader->uniformMatVP >= 0)
		{
			Mat4x4f vp = projectionMatrix * viewMatrix;
			glUniformMatrix4fv(shader->uniformMatVP, 1, GL_FALSE, vp.ValuePointer());
		}

		if (shader->uniformMatM >= 0)
		{
			glUniformMatrix4fv(shader->uniformMatM, 1, GL_FALSE, modelMatrix.ValuePointer());
		}

		if (shader->uniformMatV >= 0)
		{
			glUniformMatrix4fv(shader->uniformMatV, 1, GL_FALSE, viewMatrix.ValuePointer());
		}

		if (shader->uniformMatP >= 0)
		{
			glUniformMatrix4fv(shader->uniformMatP, 1, GL_FALSE, projectionMatrix.ValuePointer());
		}

		glBindVertexArray(mesh.vertexArrayObject);

		glDrawElements(mesh.primitiveMode, mesh.indexCount, mesh.indexElementType, nullptr);
	}

	glBindVertexArray(0);

	this->drawCalls.Clear();
}

void Renderer::AttachTarget(Window* window)
{
	this->targetWindow = window;
}

void Renderer::SetActiveCamera(Camera* camera)
{
	this->activeCamera = camera;
}

void Renderer::CreateDrawCalls(Scene* scene)
{
	Engine* engine = Engine::GetInstance();
	ResourceManager* rm = engine->GetResourceManager();

	Mat4x4f cameraTransform = scene->GetWorldTransform(activeCamera->GetSceneObjectId());
	Vec3f cameraPosition = (cameraTransform * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();
	Vec3f cameraForward = (cameraTransform * Vec4f(0.0f, 0.0f, -1.0f, 0.0f)).xyz();
	float farPlane = activeCamera->farClipDistance;

	for (unsigned index = 0; index < objectCount; ++index)
	{
		// Object is in potentially visible set
		if (cullingState[index] != 0)
		{
			RenderObject& obj = objects[index];
			Material& material = rm->GetMaterial(obj.materialId);
			Shader* shader = rm->GetShader(material.shaderId);

			Mat4x4f objTransform = scene->GetWorldTransform(obj.sceneObjectId);
			Vec3f objPosition = (objTransform * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();

			float depth = Vec3f::Dot(objPosition - cameraPosition, cameraForward) / farPlane;

			if (depth > 1.0f)
				depth = 1.0f;
			else if (depth < 0.0f)
				depth = 0.0f;

			RenderTransparencyType transparency = shader->transparencyType;

			// Add draw call

			uint64_t order = 0;

			{
				using namespace RenderOrder;
				const RenderOrderConfiguration& conf = this->renderOrderConfiguration;
				
				conf.viewportIndex.AssignValue(order, FullscreenViewport);
				conf.viewportLayer.AssignValue(order, obj.layer);
				conf.transparencyType.AssignValue(order, static_cast<uint64_t>(transparency));
				conf.command.AssignValue(order, DrawCommand);

				switch (transparency)
				{
					case RenderTransparencyType::Opaque:
					case RenderTransparencyType::AlphaTest:
					{
						float scaledDepth = ((1 << conf.opaqueDepth.bits) - 1) * depth;
						uint64_t intDepth = static_cast<uint64_t>(scaledDepth);

						conf.opaqueDepth.AssignValue(order, intDepth);
						conf.opaqueMaterialId.AssignValue(order, obj.materialId);
					}
						break;

					case RenderTransparencyType::TransparentMix:
					case RenderTransparencyType::TransparentAdd:
					case RenderTransparencyType::TransparentSub:

						break;
				}
			}

			DrawCall drawCall;
			drawCall.orderKey = order;
			drawCall.renderObjectIndex = index;
			drawCalls.PushBack(drawCall);
		}
	}
}

void Renderer::UpdateBoundingBoxes(Scene* scene)
{
	Engine* engine = Engine::GetInstance();
	ResourceManager* rm = engine->GetResourceManager();

	for (unsigned i = 0; i < objectCount; ++i)
	{
		const Mesh& mesh = rm->meshes.Get(objects[i].meshId);

		const Mat4x4f& matrix = scene->GetWorldTransform(objects[i].sceneObjectId);
		boundingBoxes[i] = mesh.bounds.Transform(matrix);
	}
}

void Renderer::Reallocate()
{
	unsigned int newAllocatedCount;

	if (allocatedCount == 0)
	{
		newAllocatedCount = 1023;
	}
	else
	{
		newAllocatedCount = allocatedCount * 2 + 1;
	}

	unsigned int* newIndexList = new unsigned int[newAllocatedCount + 1];
	RenderObject* newObjects = new RenderObject[newAllocatedCount];
	boundingBoxes = new BoundingBox[newAllocatedCount];
	cullingState = new unsigned char[newAllocatedCount];

	// We have old data
	if (allocatedCount > 0)
	{
		// Copy old data to new buffers
		std::memcpy(newIndexList, this->indexList, allocatedCount);
		std::memcpy(newObjects, this->objects, allocatedCount);

		// Delete old buffers
		delete[] this->indexList;
		delete[] this->objects;

		// We can delete bounding box data without copying, because it is recreated on every frame
		delete[] boundingBoxes;
		delete[] cullingState;
	}

	this->indexList = newIndexList;
	this->objects = newObjects;

	allocatedCount = newAllocatedCount;
}

unsigned int Renderer::AddRenderObject()
{
	unsigned int id;

	if (freeListFirst == 0)
	{
		if (objectCount == allocatedCount)
		{
			this->Reallocate();
		}

		// If there are no freelist entries, first <objectCount> indices must be in use
		id = objectCount + 1;
		indexList[id] = objectCount;

		++objectCount;
	}
	else
	{
		id = freeListFirst;
		indexList[id] = objectCount;

		freeListFirst = indexList[freeListFirst];
	}

	return id;
}

void Renderer::RemoveRenderObject(unsigned int id)
{
	assert(id != 0);

	// Put last object in removed objects place
	if (indexList[id] < objectCount - 1)
	{
		objects[indexList[id]] = objects[objectCount - 1];
	}

	indexList[id] = freeListFirst;
	freeListFirst = id;

	--objectCount;
}
