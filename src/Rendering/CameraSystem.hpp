#pragma once

#include "Engine/ComponentSystemDefaultImpl.hpp"

#include "Math/Projection.hpp"

class Allocator;

using CameraId = ComponentSystemDefaultImpl<ProjectionParameters>::ComponentId;

class CameraSystem : public ComponentSystemDefaultImpl<ProjectionParameters>
{
private:
	static const size_t ProjectionTypeCount = 2;
	static const char* ProjectionTypeNames[ProjectionTypeCount];
	static const char* ProjectionTypeDisplayNames[ProjectionTypeCount];

public:
	CameraSystem(Allocator* allocator);

	static const char* GetProjectionTypeName(ProjectionType type);
	static const char* GetProjectionTypeDisplayName(ProjectionType type);
};
