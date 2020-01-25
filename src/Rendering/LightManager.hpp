#pragma once

#include "Scene/ITransformUpdateReceiver.hpp"

#include "Rendering/Light.hpp"
#include "Entity/Entity.hpp"
#include "Math/Mat3x3.hpp"
#include "Math/Vec3.hpp"
#include "Core/HashMap.hpp"
#include "Core/Array.hpp"
#include "Math/Frustum.hpp"

class Allocator;

struct LightId
{
	unsigned int i;

	bool IsNull() const { return i == 0; }
};

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
		return pair != nullptr ? pair->value : LightId{};
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

	bool GetShadowCasting(LightId id) const { return data.shadowCasting[id.i]; }
	void SetShadowCasting(LightId id, bool shadowCasting) { data.shadowCasting[id.i] = shadowCasting; }

	LightId GetDirectionalLights();
	Array<LightId> GetNonDirectionalLightsWithinFrustum(const FrustumPlanes& frustum);
};
