#pragma once

#include "Mat4x4.hpp"
#include "Vec3.hpp"
#include "Vec2.hpp"
#include "Entity.hpp"
#include "MeshData.hpp"
#include "MaterialData.hpp"

#include "RenderCommandList.hpp"
#include "RenderOrder.hpp"

#include "Array.hpp"
#include "HashMap.hpp"

struct BoundingBox;
struct BitPack;
class Camera;
class Window;
class World;
class Scene;

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

struct RendererFramebuffer
{
	static const unsigned int MaxTextureCount = 4;

	unsigned int framebuffer;
	unsigned int textures[MaxTextureCount];
	unsigned int textureCount;
	Vec2i resolution;
};

struct RendererViewportTransform
{
	Mat4x4f view;
	Mat4x4f projection;
	Mat4x4f viewProjection;
};

class Renderer
{
private:
	static const unsigned int MaxViewportCount = 8;

	static const unsigned int NormalTextureIdx = 0;
	static const unsigned int AlbedoSpecTextureIdx = 1;
	static const unsigned int DepthTextureIdx = 2;

	struct LightingData
	{
		MeshId dirMesh;
		MeshId pointMesh;
		MeshId spotMesh;

		unsigned int dirShaderHash;

		Vec3f lightPos;
		Vec3f lightDir;
		Vec3f lightCol;
		MaterialId shadowMaterial;
	}
	lightingData;

	RendererFramebuffer gbuffer;
	RendererFramebuffer shadowBuffer;

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

	Camera* overrideRenderCamera;
	Camera* overrideCullingCamera;

	RenderCommandList commandList;
	Array<BitPack> objectVisibility;

	RendererViewportTransform viewportTransforms[MaxViewportCount];
	unsigned int lightViewportIndex;
	unsigned int fullscreenViewportIndex;

	void Reallocate(unsigned int required);

	void UpdateTransforms(Scene* scene);
	void UpdateBoundingBoxes(Scene* scene);
	void CreateDrawCalls(Scene* scene);

	bool ParseControlCommand(uint64_t orderKey);

	Camera* GetRenderCamera(Scene* scene);
	Camera* GetCullingCamera(Scene* scene);
	
public:
	Renderer();
	~Renderer();

	void Initialize(Window* window);
	void Deinitialize();

	void SetRenderCameraOverride(Camera* renderCamera) { overrideRenderCamera = renderCamera; }
	void SetCullingCameraOverride(Camera* cullingCamera) { overrideCullingCamera = cullingCamera; }

	// Render the specified scene to the active OpenGL context
	void Render(Scene* scene);

	void NotifyUpdatedTransforms(unsigned int count, Entity* entities, Mat4x4f* transforms);

	// Render object management

	RenderObjectId Lookup(Entity e)
	{
		HashMap<unsigned int, RenderObjectId>::KeyValuePair* pair = entityMap.Lookup(e.id);
		return pair != nullptr ? pair->value : RenderObjectId{};
	}

	RenderObjectId AddRenderObject(Entity entity);
	void AddRenderObject(unsigned int count, Entity* entities, RenderObjectId* renderObjectIdsOut);

	// Render object property management

	void SetMeshId(RenderObjectId id, MeshId meshId) { data.mesh[id.i] = meshId; }
	MeshId GetMeshId(RenderObjectId id) { return data.mesh[id.i]; }

	void SetOrderData(RenderObjectId id, const RenderOrderData& order)
	{
		data.order[id.i] = order;
	}
};
