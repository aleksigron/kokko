#pragma once

#include "Core/Array.hpp"
#include "Core/BitPack.hpp"
#include "Core/HashMap.hpp"

#include "Entity/Entity.hpp"

#include "Math/Mat4x4.hpp"
#include "Math/Vec3.hpp"
#include "Math/Vec2.hpp"

#include "Resources/MeshData.hpp"
#include "Resources/MaterialData.hpp"
#include "Resources/ShaderId.hpp"

#include "Rendering/Light.hpp"
#include "Rendering/RenderCommandList.hpp"
#include "Rendering/RendererData.hpp"
#include "Rendering/RenderOrder.hpp"

#include "Scene/ITransformUpdateReceiver.hpp"

class Allocator;
class Camera;
class LightManager;
class ShaderManager;
class RenderDevice;
class Scene;
class Window;

struct BoundingBox;
struct RendererFramebuffer;
struct RendererViewport;
struct MaterialData;
struct ShaderData;
struct ProjectionParameters;

class Renderer : public ITransformUpdateReceiver
{
private:
	static const unsigned int MaxViewportCount = 8;

	static const unsigned int AlbedoTextureIdx = 0;
	static const unsigned int NormalTextureIdx = 1;
	static const unsigned int MaterialTextureIdx = 2;
	static const unsigned int DepthTextureIdx = 3;

	static const unsigned int FramebufferIndexGBuffer = 0;
	static const unsigned int FramebufferIndexShadow = 1;

	Allocator* allocator;

	RenderDevice* device;

	RendererFramebuffer* framebufferData;
	unsigned int framebufferCount;

	RendererViewport* viewportData;
	unsigned int viewportCount;
	unsigned int viewportIndexFullscreen;

	MeshId lightingMesh;
	ShaderId lightingShader;
	MaterialId shadowMaterial;
	unsigned int lightingUniformBufferId;
	unsigned int objectUniformBufferId;

	struct InstanceData
	{
		unsigned int count;
		unsigned int allocated;
		void *buffer;

		Entity* entity;
		MeshId* mesh;
		RenderOrderData* order;
		BoundingBox* bounds;
		Mat4x4f* transform;
	}
	data;

	HashMap<unsigned int, RenderObjectId> entityMap;

	RenderOrderConfiguration renderOrder;

	LightManager* lightManager;
	ShaderManager* shaderManager;

	Camera* overrideRenderCamera;
	Camera* overrideCullingCamera;

	RenderCommandList commandList;
	Array<BitPack> objectVisibility;

	Array<LightId> lightResultArray;

	Entity skyboxEntity;

	void ReallocateRenderObjects(unsigned int required);

	void BindMaterialTextures(const MaterialData& material) const;
	void BindLightingTextures(const ShaderData& shader) const;
	void UpdateLightingDataToUniformBuffer(
		unsigned char* toBuffer, const ProjectionParameters& projection, const Scene* scene);

	void PopulateCommandList(Scene* scene);

	bool ParseControlCommand(uint64_t orderKey);

	Camera* GetRenderCamera(Scene* scene);
	Camera* GetCullingCamera(Scene* scene);
	
public:
	Renderer(Allocator* allocator, RenderDevice* renderDevice,
		LightManager* lightManager, ShaderManager* shaderManager);
	~Renderer();

	void Initialize(Window* window);
	void Deinitialize();

	void SetRenderCameraOverride(Camera* renderCamera) { overrideRenderCamera = renderCamera; }
	void SetCullingCameraOverride(Camera* cullingCamera) { overrideCullingCamera = cullingCamera; }

	// Render the specified scene to the active OpenGL context
	void Render(Scene* scene);

	virtual void NotifyUpdatedTransforms(unsigned int count, const Entity* entities, const Mat4x4f* transforms);

	// Render object management

	RenderObjectId Lookup(Entity e)
	{
		HashMap<unsigned int, RenderObjectId>::KeyValuePair* pair = entityMap.Lookup(e.id);
		return pair != nullptr ? pair->second : RenderObjectId{};
	}

	RenderObjectId AddRenderObject(Entity entity);
	void AddRenderObject(unsigned int count, const Entity* entities, RenderObjectId* renderObjectIdsOut);

	// Render object property management

	void SetMeshId(RenderObjectId id, MeshId meshId) { data.mesh[id.i] = meshId; }
	MeshId GetMeshId(RenderObjectId id) { return data.mesh[id.i]; }

	void SetOrderData(RenderObjectId id, const RenderOrderData& order)
	{
		data.order[id.i] = order;
	}
};
