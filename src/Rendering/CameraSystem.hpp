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

public:
	CameraSystem(Allocator* allocator);
	~CameraSystem();

	CameraId Lookup(Entity e);

	CameraId AddCameraComponent(Entity e);
	void RemoveCameraComponent(CameraId id);

	Entity GetCameraEntity(CameraId id) const;

	const ProjectionParameters& GetProjectionParameters(CameraId id) const;
	void SetProjectionParameters(CameraId id, const ProjectionParameters& params);
};
