#pragma once

#include "Mat4x4.hpp"
#include "RenderPipeline.hpp"
#include "RenderOrder.hpp"
#include "RenderData.hpp"
#include "FrustumCulling.hpp"
#include "MeshData.hpp"
#include "Array.hpp"
#include "HashMap.hpp"

struct BoundingBox;
struct CullStatePacked16;
class Camera;
class Window;
class World;
class Scene;

struct RenderObjectId
{
	static const RenderObjectId Null;

	unsigned int i;
};

class Renderer
{
private:
	struct InstanceData
	{
		unsigned int count;
		unsigned int allocated;
		void *buffer;

		Entity* entity;
		MeshId* mesh;
		unsigned int* material;
		SceneLayer* layer;
		CullStatePacked16* cullState;
		BoundingBox* bounds;
		Mat4x4f* transform;
	}
	data;

	HashMap<unsigned int, RenderObjectId> entityMap;

	Camera* overrideRenderCamera;
	Camera* overrideCullingCamera;

	Array<RenderCommand> commands;

	RenderPipeline pipeline;

	void Reallocate(unsigned int required);

	void UpdateTransforms(Scene* scene);
	void UpdateBoundingBoxes(Scene* scene);
	void CreateDrawCalls(Scene* scene);

	Camera* GetRenderCamera(Scene* scene);
	Camera* GetCullingCamera(Scene* scene);
	
public:
	Renderer();
	~Renderer();

	void SetRenderCameraOverride(Camera* renderCamera) { overrideRenderCamera = renderCamera; }
	void SetCullingCameraOverride(Camera* cullingCamera) { overrideCullingCamera = cullingCamera; }

	// Render the specified scene to the active OpenGL context
	void Render(Scene* scene);

	void NotifyUpdatedTransforms(unsigned int count, Entity* entities, Mat4x4f* transforms);

	// Render object management

	RenderObjectId Lookup(Entity e)
	{
		HashMap<unsigned int, RenderObjectId>::KeyValuePair* pair = entityMap.Lookup(e.id);
		return pair != nullptr ? pair->value : RenderObjectId::Null;
	}

	RenderObjectId AddRenderObject(Entity entity);
	void AddRenderObject(unsigned int count, Entity* entities, RenderObjectId* renderObjectIdsOut);

	// Render object property management

	void SetMeshId(RenderObjectId id, MeshId meshId) { data.mesh[id.i] = meshId; }
	MeshId GetMeshId(RenderObjectId id) { return data.mesh[id.i]; }

	void SetMaterialId(RenderObjectId id, unsigned int materialId) { data.material[id.i] = materialId; }
	unsigned int GetMaterialId(RenderObjectId id) { return data.material[id.i]; }

	void SetSceneLayer(RenderObjectId id, SceneLayer layer) { data.layer[id.i] = layer; }
	SceneLayer GetSceneLayer(RenderObjectId id) { return data.layer[id.i]; }
};
