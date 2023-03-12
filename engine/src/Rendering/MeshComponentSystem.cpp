#include "Rendering/MeshComponentSystem.hpp"

#include <cassert>

#include "Engine/Entity.hpp"

#include "Math/BoundingBox.hpp"
#include "Math/Mat4x4.hpp"

#include "Resources/MaterialData.hpp"
#include "Resources/MeshManager.hpp"

namespace kokko
{

const MeshComponentId MeshComponentId::Null = MeshComponentId{ 0 };

MeshComponentSystem::MeshComponentSystem(Allocator* allocator, MeshManager* meshManager) :
	allocator(allocator),
	meshManager(meshManager),
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
				const BoundingBox* bounds = meshManager->GetBoundingBox(meshId);
				data.bounds[dataIdx] = bounds->Transform(transforms[entityIdx]);
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
		data.material[id] = MaterialId::Null;
		data.transparency[id] = TransparencyType::Opaque;
		data.bounds[id] = BoundingBox();
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

void MeshComponentSystem::SetMeshId(MeshComponentId id, MeshId meshId)
{
	data.mesh[id.i] = meshId;
}

MeshId MeshComponentSystem::GetMeshId(MeshComponentId id) const
{
	return data.mesh[id.i];
}

void MeshComponentSystem::SetMaterial(MeshComponentId id, MaterialId materialId, TransparencyType transparency)
{
	data.material[id.i] = materialId;
	data.transparency[id.i] = transparency;
}

MaterialId MeshComponentSystem::GetMaterialId(MeshComponentId id) const
{
	return data.material[id.i];
}

TransparencyType MeshComponentSystem::GetTransparencyType(MeshComponentId id) const
{
	return data.transparency[id.i];
}

void MeshComponentSystem::Reallocate(unsigned int required)
{
	if (required <= data.allocated)
		return;

	required = static_cast<unsigned int>(Math::UpperPowerOfTwo(required));

	// Reserve same amount in entity map
	entityMap.Reserve(required);

	InstanceData newData;
	unsigned int bytes = required * (sizeof(Entity) + sizeof(MeshId) + sizeof(MaterialId) +
		sizeof(TransparencyType) + sizeof(BoundingBox) + sizeof(Mat4x4f));

	newData.buffer = this->allocator->Allocate(bytes, "MeshComponentSystem.data.buffer");
	newData.count = data.count;
	newData.allocated = required;

	newData.entity = static_cast<Entity*>(newData.buffer);
	newData.mesh = reinterpret_cast<MeshId*>(newData.entity + required);
	newData.material = reinterpret_cast<MaterialId*>(newData.mesh + required);
	newData.transparency = reinterpret_cast<TransparencyType*>(newData.material + required);
	newData.bounds = reinterpret_cast<BoundingBox*>(newData.transparency + required);
	newData.transform = reinterpret_cast<Mat4x4f*>(newData.bounds + required);

	if (data.buffer != nullptr)
	{
		std::memcpy(newData.entity, data.entity, data.count * sizeof(Entity));
		std::memcpy(newData.mesh, data.mesh, data.count * sizeof(MeshId));
		std::memcpy(newData.material, data.material, data.count * sizeof(MaterialId));
		std::memcpy(newData.transparency, data.transparency, data.count * sizeof(TransparencyType));
		std::memcpy(newData.bounds, data.bounds, data.count * sizeof(BoundingBox));
		std::memcpy(newData.transform, data.transform, data.count * sizeof(Mat4x4f));

		this->allocator->Deallocate(data.buffer);
	}

	data = newData;
}

}