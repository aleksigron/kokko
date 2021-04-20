#include "Rendering/CameraSystem.hpp"

#include <cassert>

const CameraId CameraId::Null = CameraId{ 0 };

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

	HashMap<unsigned int, CameraId>::KeyValuePair* pair = entityMap.Lookup(entity.id);
	if (pair != nullptr)
		entityMap.Remove(pair);

	if (id.i + 1 < cameraData.GetCount()) // Need to swap
	{
		const CameraData& swap = cameraData.GetBack();

		HashMap<unsigned int, CameraId>::KeyValuePair* swapKv = entityMap.Lookup(swap.entity.id);
		if (swapKv != nullptr)
			swapKv->second = CameraId{ id.i };

		cameraData[id.i] = swap;
	}

	cameraData.PopBack();
}

const ProjectionParameters& CameraSystem::GetProjectionParameters(CameraId id) const
{
	return cameraData[id.i].projectionParams;
}

void CameraSystem::SetProjectionParameters(CameraId id, const ProjectionParameters& params)
{
	cameraData[id.i].projectionParams = params;
}
