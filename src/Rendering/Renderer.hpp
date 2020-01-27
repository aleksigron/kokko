#pragma once

#include "Scene/ITransformUpdateReceiver.hpp"

#include "Math/Mat4x4.hpp"
#include "Math/Vec3.hpp"
#include "Math/Vec2.hpp"
#include "Entity/Entity.hpp"
#include "Math/Frustum.hpp"
#include "Resources/MeshData.hpp"
#include "Resources/MaterialData.hpp"

#include "Rendering/RenderCommandList.hpp"
#include "Rendering/RenderOrder.hpp"

#include "Core/Array.hpp"
#include "Core/HashMap.hpp"

class Allocator;

struct BoundingBox;
struct BitPack;
class Camera;
class Window;
class World;
class Scene;
class LightManager;

struct RenderObjectId
{
	unsigned int i;

	bool IsNull() const { return i == 0; }
};

struct RenderOrderData
{
	MaterialId material;
	TransparencyType transparency;
};

struct RendererViewport
{
	Vec3f position;
	Vec3f forward;

	float farMinusNear;
	float minusNear;

	Mat4x4f view;
	Mat4x4f projection;
	Mat4x4f viewProjection;

	FrustumPlanes frustum;

	unsigned int framebufferIndex;
};

struct RendererFramebuffer
{
	static const unsigned int MaxTextureCount = 4;

	unsigned int framebuffer;
	unsigned int textures[MaxTextureCount];
	unsigned int textureCount;
	Vec2i resolution;
	bool used;
};

class Renderer : public ITransformUpdateReceiver
{
private:
	static const unsigned int MaxViewportCount = 8;

	static const unsigned int NormalTextureIdx = 0;
	static const unsigned int AlbedoSpecTextureIdx = 1;
	static const unsigned int DepthTextureIdx = 2;

	Allocator* allocator;

	struct LightingData
	{
		MeshId dirMesh;
		MeshId pointMesh;
		MeshId spotMesh;

		unsigned int dirShaderHash;

		MaterialId shadowMaterial;
	}
	lightingData;

	RendererFramebuffer* framebufferData;
	unsigned int framebufferCount;
	static const unsigned int FramebufferIndexGBuffer = 0;

	RendererViewport* viewportData;
	unsigned int viewportCount;
	unsigned int viewportIndexFullscreen;

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
	Vec3f primaryDirectionalLightDirection;

	Camera* overrideRenderCamera;
	Camera* overrideCullingCamera;

	RenderCommandList commandList;
	Array<BitPack> objectVisibility;

	unsigned int GetDepthFramebufferOfSize(const Vec2i& size);
	void ClearFramebufferUsageFlags();

	void ReallocateRenderObjects(unsigned int required);

	void PopulateCommandList(Scene* scene);

	bool ParseControlCommand(uint64_t orderKey);

	Camera* GetRenderCamera(Scene* scene);
	Camera* GetCullingCamera(Scene* scene);
	
public:
	Renderer(Allocator* allocator, LightManager* lightManager);
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
