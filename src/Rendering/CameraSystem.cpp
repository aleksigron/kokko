#include "Rendering/CameraSystem.hpp"

const CameraId CameraId::Null = CameraId{ 0 };

CameraSystem::CameraSystem(Allocator* allocator) :
	cameraData(allocator),
	entityMap(allocator)
{
	cameraData.Reserve(16);
	cameraData.PushBack(CameraData{ Entity::Null });
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
	return CameraId();
}

void CameraSystem::RemoveCameraComponent(CameraId id)
{
}

const ProjectionParameters& CameraSystem::GetProjectionParameters(CameraId id) const
{
	return cameraData[id.i].projectionParams;
}

void CameraSystem::SetProjectionParameters(CameraId id, const ProjectionParameters& params)
{
	cameraData[id.i].projectionParams = params;
}
