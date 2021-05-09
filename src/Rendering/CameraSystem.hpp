#pragma once

#include "Engine/ComponentSystemDefaultImpl.hpp"

#include "Math/Projection.hpp"

#include "Rendering/CameraId.hpp"

class Allocator;

class CameraSystem : public ComponentSystemDefaultImpl<ProjectionParameters, CameraId>
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
