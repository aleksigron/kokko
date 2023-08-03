#pragma once

#include "Core/HashMap.hpp"

#include "Graphics/TransformUpdateReceiver.hpp"

#include "Rendering/TransparencyType.hpp"

class Allocator;

struct Entity;
struct Mat4x4f;

namespace kokko
{

class MeshManager;
class Renderer;

struct AABB;
struct MaterialId;
struct MeshId;

struct MeshComponentId
{
	unsigned int i;

	bool operator==(MeshComponentId other) const { return i == other.i; }
	bool operator!=(MeshComponentId other) const { return !operator==(other); }

	static const MeshComponentId Null;
};

class MeshComponentSystem : public TransformUpdateReceiver
{
	friend class kokko::Renderer;

public:
	MeshComponentSystem(Allocator* allocator, MeshManager* meshManager);
	~MeshComponentSystem();

	virtual void NotifyUpdatedTransforms(size_t count, const Entity* entities, const Mat4x4f* transforms) override;

	MeshComponentId Lookup(Entity entity);

	MeshComponentId AddComponent(Entity entity);
	void AddComponents(unsigned int count, const Entity* entities, MeshComponentId* idsOut);

	void RemoveComponent(MeshComponentId id);
	void RemoveAll();

	void SetMeshId(MeshComponentId id, MeshId meshId);
	MeshId GetMeshId(MeshComponentId id) const;

	void SetMaterial(MeshComponentId id, MaterialId materialId, TransparencyType transparency);
	MaterialId GetMaterialId(MeshComponentId id) const;
	TransparencyType GetTransparencyType(MeshComponentId id) const;

private:
	void Reallocate(unsigned int required);

	Allocator* allocator;
	MeshManager* meshManager;

	struct InstanceData
	{
		unsigned int count;
		unsigned int allocated;
		void* buffer;

		Entity* entity;
		MeshId* mesh;
		MaterialId* material;
		TransparencyType* transparency;
		AABB* bounds;
		Mat4x4f* transform;
	}
	data;

	// Look up table from entity to component id / index
	HashMap<unsigned int, unsigned int> entityMap;
};

}
