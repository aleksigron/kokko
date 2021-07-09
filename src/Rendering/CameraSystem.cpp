#include "Rendering/CameraSystem.hpp"

#include <cassert>

const char* CameraSystem::ProjectionTypeNames[] = { "perspective", "orthographic" };
const char* CameraSystem::ProjectionTypeDisplayNames[] = { "Perspective", "Orthographic" };

CameraSystem::CameraSystem(Allocator* allocator) :
	ComponentSystemDefaultImpl(allocator)
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
