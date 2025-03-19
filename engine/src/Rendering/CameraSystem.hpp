#pragma once

#include <cstddef>

#include "Core/Array.hpp"
#include "Core/HashMap.hpp"
#include "Core/BitPack.hpp"

#include "Engine/Entity.hpp"

#include "Graphics/TransformUpdateReceiver.hpp"

#include "Math/Frustum.hpp"
#include "Math/Mat3x3.hpp"
#include "Math/Vec3.hpp"

#include "Math/Projection.hpp"

namespace kokko
{

class Allocator;

struct CameraComponent
{
	ProjectionParameters projection;
	float exposure;
};

struct CameraId
{
	unsigned int i;

	bool operator==(CameraId other) { return i == other.i; }
	bool operator!=(CameraId other) { return !operator==(other); }

	static const CameraId Null;
};

class CameraSystem
{
public:
	CameraSystem(Allocator* allocator);
	~CameraSystem();

	static const char* GetProjectionTypeName(ProjectionType type);
	static const char* GetProjectionTypeDisplayName(ProjectionType type);

	CameraId Lookup(Entity e)
	{
		auto* pair = entityMap.Lookup(e.id);
		return pair != nullptr ? pair->second : CameraId{};
	}

	CameraId AddCamera(Entity entity);
	void AddCamera(unsigned int count, const Entity* entities, CameraId* cameraIdsOut);

	void RemoveCamera(CameraId id);

	void RemoveAll();

	Entity GetEntity(CameraId id) const { return data.entity[id.i]; }

	const ProjectionParameters& GetProjection(CameraId id) const;
	void SetProjection(CameraId id, const ProjectionParameters& parameters);

	float GetExposure(CameraId id) const;
	void SetExposure(CameraId id, float exposure);

	Entity GetActiveCamera() const;
	void SetActiveCamera(Entity entity);

private:
	static const size_t ProjectionTypeCount = 2;
	static const char* ProjectionTypeNames[ProjectionTypeCount];
	static const char* ProjectionTypeDisplayNames[ProjectionTypeCount];

	Allocator* allocator;

	HashMap<unsigned int, CameraId> entityMap;

	struct InstanceData
	{
		unsigned int count;
		unsigned int allocated;
		void* buffer;

		Entity* entity;
		CameraComponent* components;
	}
	data;

	Entity activeCamera;

	void Reallocate(unsigned int required);
};

} // namespace kokko
