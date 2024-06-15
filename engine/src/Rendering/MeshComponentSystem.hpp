#pragma once

#include "Core/ArrayView.hpp"
#include "Core/HashMap.hpp"

#include "Graphics/TransformUpdateReceiver.hpp"

#include "Rendering/TransparencyType.hpp"

class Allocator;

struct Entity;
struct Mat4x4f;

namespace kokko
{

class ModelManager;
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
	MeshComponentSystem(Allocator* allocator, ModelManager* modelManager);
	~MeshComponentSystem();

	virtual void NotifyUpdatedTransforms(size_t count, const Entity* entities, const Mat4x4f* transforms) override;

	MeshComponentId Lookup(Entity entity);

	MeshComponentId AddComponent(Entity entity);
	void AddComponents(unsigned int count, const Entity* entities, MeshComponentId* idsOut);

	void RemoveComponent(MeshComponentId id);
	void RemoveAll();

	void SetMesh(MeshComponentId id, MeshId meshId, uint32_t partCount);
	MeshId GetMeshId(MeshComponentId id) const;

	void SetMaterial(MeshComponentId id, uint32_t partIndex, MaterialId materialId, TransparencyType transparency);
	ArrayView<const MaterialId> GetMaterialIds(MeshComponentId id) const;
	ArrayView<const TransparencyType> GetTransparencyTypes(MeshComponentId id) const;

private:
	void Reallocate(unsigned int required);

	Allocator* allocator;
	ModelManager* modelManager;

	template <typename ItemType, typename SizeType, SizeType MaxCount>
	class CompactStorage
	{
	public:
		CompactStorage();
		void Resize(SizeType newCount);
		ArrayView<ItemType> GetDataView();
		ArrayView<const ItemType> GetDataView() const;

	private:
		SizeType count;
		ItemType data[MaxCount];
	};

	using MaterialStorage = CompactStorage<MaterialId, uint16_t, 7>;
	using TransparencyStorage = CompactStorage<TransparencyType, uint8_t, 7>;

	struct InstanceData
	{
		unsigned int count;
		unsigned int allocated;
		void* buffer;

		Entity* entity;
		MeshId* mesh;
		MaterialStorage* material;
		TransparencyStorage* transparency;
		AABB* bounds;
		Mat4x4f* transform;
	}
	data;

	// Look up table from entity to component id / index
	HashMap<unsigned int, unsigned int> entityMap;
};

}
