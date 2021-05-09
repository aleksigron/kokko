#include "Rendering/CameraSystem.hpp"

#include <cassert>

const CameraId CameraId::Null = CameraId{ 0 };
const char* CameraSystem::ProjectionTypeNames[] = { "perspective", "orthographic" };
const char* CameraSystem::ProjectionTypeDisplayNames[] = { "Perspective", "Orthographic" };

CameraSystem::CameraSystem(Allocator* allocator) :
	cameraData(allocator),
	entityMap(allocator)
{
	cameraData.Reserve(16);
	cameraData.PushBack(CameraData{ Entity::Null });

	entityMap.Reserve(16);
}

CameraSystem::~CameraSystem()
{
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

CameraId CameraSystem::Lookup(Entity e)
{
	HashMap<unsigned int, CameraId>::KeyValuePair* pair = entityMap.Lookup(e.id);
	return pair != nullptr ? pair->second : CameraId::Null;
}

CameraId CameraSystem::AddCameraComponent(Entity e)
{
	unsigned int id = cameraData.GetCount();

	auto mapPair = entityMap.Insert(e.id);
	mapPair->second.i = id;

	CameraData& camera = cameraData.PushBack();
	camera.entity = e;

	return CameraId{ id };
}

void CameraSystem::RemoveCameraComponent(CameraId id)
{
	assert(id != CameraId::Null);

	Entity entity = cameraData[id.i].entity;

	auto* pair = entityMap.Lookup(entity.id);
	if (pair != nullptr)
		entityMap.Remove(pair);

	if (id.i + 1 < cameraData.GetCount()) // Need to swap
	{
		const CameraData& swap = cameraData.GetBack();

		auto* swapKv = entityMap.Lookup(swap.entity.id);
		if (swapKv != nullptr)
			swapKv->second = id;

		cameraData[id.i] = swap;
	}

	cameraData.PopBack();
}

void CameraSystem::RemoveAll()
{
	entityMap.Clear();
	cameraData.Resize(1);
}

const ProjectionParameters& CameraSystem::GetProjectionParameters(CameraId id) const
{
	return cameraData[id.i].projectionParams;
}

void CameraSystem::SetProjectionParameters(CameraId id, const ProjectionParameters& params)
{
	cameraData[id.i].projectionParams = params;
}
