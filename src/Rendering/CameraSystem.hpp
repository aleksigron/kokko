#pragma once

#include "Core/Array.hpp"
#include "Core/HashMap.hpp"

#include "Entity/Entity.hpp"

#include "Math/Projection.hpp"

#include "Rendering/CameraId.hpp"

class Allocator;

class CameraSystem
{
private:
	struct CameraData
	{
		Entity entity;
		ProjectionParameters projectionParams;
	};

	Array<CameraData> cameraData;
	HashMap<unsigned int, CameraId> entityMap;

	static const size_t ProjectionTypeCount = 2;
	static const char* ProjectionTypeNames[ProjectionTypeCount];
	static const char* ProjectionTypeDisplayNames[ProjectionTypeCount];

public:
	CameraSystem(Allocator* allocator);
	~CameraSystem();

	static const char* GetProjectionTypeName(ProjectionType type);
	static const char* GetProjectionTypeDisplayName(ProjectionType type);

	CameraId Lookup(Entity e);

	CameraId AddCameraComponent(Entity e);
	void RemoveCameraComponent(CameraId id);

	void RemoveAll();

	const ProjectionParameters& GetProjectionParameters(CameraId id) const;
	void SetProjectionParameters(CameraId id, const ProjectionParameters& params);
};
