#include "Rendering/CameraSystem.hpp"

#include <cassert>

#include "Rendering/CameraParameters.hpp"

const CameraId CameraId::Null = CameraId{ 0 };
const char* CameraSystem::ProjectionTypeNames[] = { "perspective", "orthographic" };
const char* CameraSystem::ProjectionTypeDisplayNames[] = { "Perspective", "Orthographic" };

CameraSystem::CameraSystem(Allocator* allocator) :
	allocator(allocator),
	entityMap(allocator),
	activeCamera(Entity::Null)
{
	data = InstanceData{};
	data.count = 1; // Reserve index 0 as CameraId::Null value

	this->Reallocate(16);
}

CameraSystem::~CameraSystem()
{
	allocator->Deallocate(data.buffer);
}

const char* CameraSystem::GetProjectionTypeName(ProjectionType type)
{
	size_t index = static_cast<size_t>(type);
	return ProjectionTypeNames[index];
}

const char* CameraSystem::GetProjectionTypeDisplayName(ProjectionType type)
{
	size_t index = static_cast<size_t>(type);
	return ProjectionTypeDisplayNames[index];
}

void CameraSystem::Reallocate(unsigned int required)
{
	if (required <= data.allocated)
		return;

	required = static_cast<unsigned int>(Math::UpperPowerOfTwo(required));

	// Reserve same amount in entity map
	entityMap.Reserve(required);

	InstanceData newData;
	unsigned int bytes = required * (sizeof(Entity) + sizeof(ProjectionParameters));

	newData.buffer = allocator->Allocate(bytes);
	newData.count = data.count;
	newData.allocated = required;

	newData.entity = static_cast<Entity*>(newData.buffer);
	newData.projection = reinterpret_cast<ProjectionParameters*>(newData.entity + required);

	if (data.buffer != nullptr)
	{
		std::memcpy(newData.entity, data.entity, data.count * sizeof(Entity));
		std::memcpy(newData.projection, data.projection, data.count * sizeof(ProjectionParameters));

		allocator->Deallocate(data.buffer);
	}

	data = newData;
}

CameraId CameraSystem::AddCamera(Entity entity)
{
	CameraId id;
	this->AddCamera(1, &entity, &id);
	return id;
}

void CameraSystem::AddCamera(unsigned int count, const Entity* entities, CameraId* cameraIdsOut)
{
	if (data.count + count > data.allocated)
		this->Reallocate(data.count + count);

	for (unsigned int i = 0; i < count; ++i)
	{
		unsigned int id = data.count + i;

		Entity e = entities[i];

		if (activeCamera == Entity::Null)
			activeCamera = e;

		auto mapPair = entityMap.Insert(e.id);
		mapPair->second.i = id;

		data.entity[id] = e;
		data.projection[id] = ProjectionParameters{};

		cameraIdsOut[i].i = id;
	}

	data.count += count;
}

void CameraSystem::RemoveCamera(CameraId id)
{
	assert(id != CameraId::Null);
	assert(id.i < data.count);

	// Remove from entity map
	Entity entity = data.entity[id.i];
	auto* pair = entityMap.Lookup(entity.id);
	if (pair != nullptr)
		entityMap.Remove(pair);

	// Update active camera if the active camera is removed
	if (activeCamera == entity && data.count > 2)
	{
		unsigned int swapIdx = (id.i == data.count - 1) ? 1 : data.count - 1;
		activeCamera = data.entity[swapIdx];
	}

	if (data.count > 2 && id.i + 1 < data.count) // We need to swap another object
	{
		unsigned int swapIdx = data.count - 1;

		// Update the swapped objects id in the entity map
		auto* swapKv = entityMap.Lookup(data.entity[swapIdx].id);
		if (swapKv != nullptr)
			swapKv->second = id;

		data.entity[id.i] = data.entity[swapIdx];
		data.projection[id.i] = data.projection[swapIdx];
	}

	--data.count;
}

void CameraSystem::RemoveAll()
{
	entityMap.Clear();
	data.count = 1;

	activeCamera = Entity::Null;
}

const ProjectionParameters& CameraSystem::GetProjection(CameraId id) const
{
	return data.projection[id.i];
}

void CameraSystem::SetProjection(CameraId id, const ProjectionParameters& parameters) const
{
	data.projection[id.i] = parameters;
}

Entity CameraSystem::GetActiveCamera() const
{
	return activeCamera;
}

void CameraSystem::SetActiveCamera(Entity entity)
{
	activeCamera = entity;
}
 