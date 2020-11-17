#pragma once

#include "Core/Array.hpp"
#include "Core/HashMap.hpp"

#include "Entity/Entity.hpp"

#include "Math/Frustum.hpp"
#include "Math/Mat3x3.hpp"
#include "Math/Vec3.hpp"

#include "Rendering/Light.hpp"

#include "Scene/ITransformUpdateReceiver.hpp"

class Allocator;

class LightManager : public ITransformUpdateReceiver
{
private:
	Allocator* allocator;

	HashMap<unsigned int, LightId> entityMap;

	struct InstanceData
	{
		unsigned int count;
		unsigned int allocated;
		void *buffer;

		Entity* entity;
		Vec3f* position;
		Mat3x3f* orientation;
		LightType* type;
		Vec3f* color;
		float* far;
		float* angle;
		bool* shadowCasting;
	}
	data;

	void Reallocate(unsigned int required);

public:
	LightManager(Allocator* allocator);
	~LightManager();

	virtual void NotifyUpdatedTransforms(unsigned int count, const Entity* entities, const Mat4x4f* transforms);

	LightId Lookup(Entity e)
	{
		HashMap<unsigned int, LightId>::KeyValuePair* pair = entityMap.Lookup(e.id);
		return pair != nullptr ? pair->second : LightId{};
	}

	LightId AddLight(Entity entity);
	void AddLight(unsigned int count, const Entity* entities, LightId* lightIdsOut);

	Entity GetEntity(LightId id) const { return data.entity[id.i]; }
	Vec3f GetPosition(LightId id) const { return data.position[id.i]; }
	Mat3x3f GetOrientation(LightId id) const { return data.orientation[id.i]; }

	LightType GetLightType(LightId id) const { return data.type[id.i]; }
	void SetLightType(LightId id, LightType type) { data.type[id.i] = type; }

	Vec3f GetColor(LightId id) const { return data.color[id.i]; }
	void SetColor(LightId id, Vec3f color) { data.color[id.i] = color; }

	float GetFarDistance(LightId id) const { return data.far[id.i]; }
	void SetFarDistance(LightId id, float distance) { data.far[id.i] = distance; }

	float GetSpotAngle(LightId id) const { return data.angle[id.i]; }
	void SetSpotAngle(LightId id, float angle) { data.angle[id.i] = angle; }

	bool GetShadowCasting(LightId id) const { return data.shadowCasting[id.i]; }
	void SetShadowCasting(LightId id, bool shadowCasting) { data.shadowCasting[id.i] = shadowCasting; }

	void GetDirectionalLights(Array<LightId>& output);
	void GetNonDirectionalLightsWithinFrustum(const FrustumPlanes& frustum, Array<LightId>& output);
};
