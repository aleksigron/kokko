#include "Rendering/MeshComponentSystem.hpp"

#include <cassert>

#include "Engine/Entity.hpp"

#include "Math/AABB.hpp"
#include "Math/Mat4x4.hpp"

#include "Resources/MaterialData.hpp"
#include "Resources/ModelManager.hpp"
#include "Resources/MeshId.hpp"

namespace kokko
{

const MeshComponentId MeshComponentId::Null = MeshComponentId{ 0 };

template <typename ItemType, typename SizeType, SizeType MaxCount>
MeshComponentSystem::CompactStorage<ItemType, SizeType, MaxCount>::CompactStorage() : count(0) { }

template <typename ItemType, typename SizeType, SizeType MaxCount>
void MeshComponentSystem::CompactStorage<ItemType, SizeType, MaxCount>::Resize(SizeType newCount)
{
	assert(newCount <= MaxCount);

	SizeType oldCount = count;
	count = newCount;

	for (SizeType i = oldCount, end = (newCount > MaxCount ? MaxCount : newCount); i != newCount; ++i)
		data[i] = ItemType{};
}

template <typename ItemType, typename SizeType, SizeType MaxCount>
ArrayView<ItemType> MeshComponentSystem::CompactStorage<ItemType, SizeType, MaxCount>::GetDataView()
{
	return ArrayView<ItemType>(count != 0 ? &data[0] : nullptr, count);
}

template <typename ItemType, typename SizeType, SizeType MaxCount>
ArrayView<const ItemType> MeshComponentSystem::CompactStorage<ItemType, SizeType, MaxCount>::GetDataView() const
{
	return ArrayView<const ItemType>(count != 0 ? &data[0] : nullptr, count);
}

template class MeshComponentSystem::CompactStorage<MaterialId, uint16_t, 7>;
template class MeshComponentSystem::CompactStorage<TransparencyType, uint8_t, 7>;

MeshComponentSystem::MeshComponentSystem(Allocator* allocator, ModelManager* modelManager) :
	allocator(allocator),
	modelManager(modelManager),
	data{},
	entityMap(allocator)
{
	data.count = 1;

	Reallocate(256);
}

MeshComponentSystem::~MeshComponentSystem()
{
	allocator->Deallocate(data.buffer);
}

void MeshComponentSystem::NotifyUpdatedTransforms(size_t count, const Entity* entities, const Mat4x4f* transforms)
{
	for (unsigned int entityIdx = 0; entityIdx < count; ++entityIdx)
	{
		Entity entity = entities[entityIdx];
		MeshComponentId id = this->Lookup(entity);

		if (id != MeshComponentId::Null)
		{
			unsigned int dataIdx = id.i;

			// Recalculate bounding box
			MeshId meshId = data.mesh[dataIdx];

			if (meshId != MeshId::Null)
			{
				const AABB& bounds = modelManager->GetModelMeshes(meshId.modelId)[meshId.meshIndex].aabb;
				data.bounds[dataIdx] = bounds.Transform(transforms[entityIdx]);
			}

			// Set world transform
			data.transform[dataIdx] = transforms[entityIdx];
		}
	}
}

MeshComponentId MeshComponentSystem::Lookup(Entity entity)
{
	auto* pair = entityMap.Lookup(entity.id);
	return pair != nullptr ? MeshComponentId{ pair->second } : MeshComponentId::Null;
}

MeshComponentId MeshComponentSystem::AddComponent(Entity entity)
{
	MeshComponentId id;
	AddComponents(1, &entity, &id);
	return id;
}

void MeshComponentSystem::AddComponents(unsigned int count, const Entity* entities, MeshComponentId* idsOut)
{
	if (data.count + count > data.allocated)
		Reallocate(data.count + count);

	for (unsigned int i = 0; i < count; ++i)
	{
		unsigned int id = data.count + i;

		Entity e = entities[i];

		auto mapPair = entityMap.Insert(e.id);
		mapPair->second = id;

		data.entity[id] = e;
		data.mesh[id] = MeshId::Null;
		data.material[id] = MaterialStorage();
		data.transparency[id] = TransparencyStorage();
		data.bounds[id] = AABB();
		data.transform[id] = Mat4x4f();

		idsOut[i].i = id;
	}

	data.count += count;
}

void MeshComponentSystem::RemoveComponent(MeshComponentId id)
{
	assert(id != MeshComponentId::Null);
	assert(id.i < data.count);

	// Remove from entity map
	Entity entity = data.entity[id.i];
	auto* pair = entityMap.Lookup(entity.id);
	if (pair != nullptr)
		entityMap.Remove(pair);

	if (data.count > 2 && id.i + 1 < data.count) // We need to swap another object
	{
		unsigned int swapIdx = data.count - 1;

		// Update the swapped objects id in the entity map
		auto* swapKv = entityMap.Lookup(data.entity[swapIdx].id);
		if (swapKv != nullptr)
			swapKv->second = id.i;

		data.entity[id.i] = data.entity[swapIdx];
		data.mesh[id.i] = data.mesh[swapIdx];
		data.material[id.i] = data.material[swapIdx];
		data.transparency[id.i] = data.transparency[swapIdx];
		data.bounds[id.i] = data.bounds[swapIdx];
		data.transform[id.i] = data.transform[swapIdx];
	}

	--data.count;
}

void MeshComponentSystem::RemoveAll()
{
	entityMap.Clear();
	data.count = 1;
}

void MeshComponentSystem::SetMesh(MeshComponentId id, MeshId meshId, uint32_t partCount)
{
	data.mesh[id.i] = meshId;
	data.material[id.i].Resize(partCount);
	data.transparency[id.i].Resize(partCount);
}

MeshId MeshComponentSystem::GetMeshId(MeshComponentId id) const
{
	return data.mesh[id.i];
}

void MeshComponentSystem::SetMaterial(MeshComponentId id, uint32_t partIndex, MaterialId materialId, TransparencyType transparency)
{
	auto materials = data.material[id.i].GetDataView();
	assert(partIndex < materials.GetCount());
	materials[partIndex] = materialId;

	auto transparencies = data.transparency[id.i].GetDataView();
	assert(partIndex < transparencies.GetCount());
	transparencies[partIndex] = transparency;
}

ArrayView<const MaterialId> MeshComponentSystem::GetMaterialIds(MeshComponentId id) const
{
	return data.material[id.i].GetDataView();
}

ArrayView<const TransparencyType> MeshComponentSystem::GetTransparencyTypes(MeshComponentId id) const
{
	return data.transparency[id.i].GetDataView();
}

void MeshComponentSystem::Reallocate(unsigned int required)
{
	if (required <= data.allocated)
		return;

	required = static_cast<unsigned int>(Math::UpperPowerOfTwo(required));

	// Reserve same amount in entity map
	entityMap.Reserve(required);

	InstanceData newData;
	unsigned int bytes = required * (sizeof(Entity) + sizeof(MeshId) + sizeof(MaterialStorage) +
		sizeof(TransparencyStorage) + sizeof(AABB) + sizeof(Mat4x4f));

	newData.buffer = this->allocator->Allocate(bytes, "MeshComponentSystem.data.buffer");
	newData.count = data.count;
	newData.allocated = required;

	newData.entity = static_cast<Entity*>(newData.buffer);
	newData.mesh = reinterpret_cast<MeshId*>(newData.entity + required);
	newData.material = reinterpret_cast<MaterialStorage*>(newData.mesh + required);
	newData.transparency = reinterpret_cast<TransparencyStorage*>(newData.material + required);
	newData.bounds = reinterpret_cast<AABB*>(newData.transparency + required);
	newData.transform = reinterpret_cast<Mat4x4f*>(newData.bounds + required);

	if (data.buffer != nullptr)
	{
		std::memcpy(newData.entity, data.entity, data.count * sizeof(Entity));
		std::memcpy(newData.mesh, data.mesh, data.count * sizeof(MeshId));
		std::memcpy(newData.material, data.material, data.count * sizeof(MaterialStorage));
		std::memcpy(newData.transparency, data.transparency, data.count * sizeof(TransparencyStorage));
		std::memcpy(newData.bounds, data.bounds, data.count * sizeof(AABB));
		std::memcpy(newData.transform, data.transform, data.count * sizeof(Mat4x4f));

		this->allocator->Deallocate(data.buffer);
	}

	data = newData;
}

}