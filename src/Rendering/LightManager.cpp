#include "Rendering/LightManager.hpp"

#include "Memory/Allocator.hpp"

#include "Math/Math.hpp"
#include "Core/BitPack.hpp"
#include "Math/Intersect3D.hpp"

LightManager::LightManager(Allocator* allocator) :
	allocator(allocator),
	entityMap(allocator)
{
	data = InstanceData{};
	data.count = 1; // Reserve index 0 as LightId::Null value

	this->Reallocate(16);
}

LightManager::~LightManager()
{
	allocator->Deallocate(data.buffer);
}

void LightManager::Reallocate(unsigned int required)
{
	if (required <= data.allocated)
		return;

	required = Math::UpperPowerOfTwo(required);

	// Reserve same amount in entity map
	entityMap.Reserve(required);

	InstanceData newData;
	unsigned int bytes = required * (sizeof(Entity) + sizeof(LightType) + sizeof(Vec3f) * 2 +
		sizeof(Mat3x3f) + sizeof(float) + sizeof(bool));

	newData.buffer = allocator->Allocate(bytes);
	newData.count = data.count;
	newData.allocated = required;

	newData.entity = static_cast<Entity*>(newData.buffer);
	newData.position = reinterpret_cast<Vec3f*>(newData.entity + required);
	newData.orientation = reinterpret_cast<Mat3x3f*>(newData.position + required);
	newData.type = reinterpret_cast<LightType*>(newData.orientation + required);
	newData.color = reinterpret_cast<Vec3f*>(newData.type + required);
	newData.far = reinterpret_cast<float*>(newData.color + required);
	newData.shadowCasting = reinterpret_cast<bool*>(newData.far + required);

	if (data.buffer != nullptr)
	{
		std::memcpy(newData.entity, data.entity, data.count * sizeof(Entity));
		std::memcpy(newData.position, data.position, data.count * sizeof(Vec3f));
		std::memcpy(newData.orientation, data.orientation, data.count * sizeof(Mat3x3f));
		std::memcpy(newData.type, data.type, data.count * sizeof(LightType));
		std::memcpy(newData.color, data.color, data.count * sizeof(Vec3f));
		std::memcpy(newData.far, data.far, data.count * sizeof(float));
		std::memcpy(newData.shadowCasting, data.shadowCasting, data.count * sizeof(bool));

		allocator->Deallocate(data.buffer);
	}

	data = newData;
}

void LightManager::NotifyUpdatedTransforms(unsigned int count, const Entity* entities, const Mat4x4f* transforms)
{
	Vec4f origin(0.0f, 0.0f, 0.0f, 1.0f);
	
	for (unsigned int entityIdx = 0; entityIdx < count; ++entityIdx)
	{
		LightId id = this->Lookup(entities[entityIdx]);

		if (id.IsNull() == false)
		{
			const Mat4x4f& t = transforms[entityIdx];
			data.position[id.i] = (t * origin).xyz();
			data.orientation[id.i] = t.Get3x3();
		}
	}
}

LightId LightManager::AddLight(Entity entity)
{
	LightId id;
	this->AddLight(1, &entity, &id);
	return id;
}

void LightManager::AddLight(unsigned int count, const Entity* entities, LightId* lightIdsOut)
{
	if (data.count + count > data.allocated)
		this->Reallocate(data.count + count);

	for (unsigned int i = 0; i < count; ++i)
	{
		unsigned int id = data.count + i;

		Entity e = entities[i];

		auto mapPair = entityMap.Insert(e.id);
		mapPair->value.i = id;

		data.entity[id] = e;

		lightIdsOut[i].i = id;
	}

	data.count += count;
}

LightId LightManager::GetDirectionalLights()
{
	for (unsigned int i = 1; i < data.count; ++i)
		if (data.type[i] == LightType::Directional)
			return LightId{ i };

	return LightId{ 0 };
}

Array<LightId> LightManager::GetNonDirectionalLightsWithinFrustum(const FrustumPlanes& frustum)
{
	Array<LightId> result(allocator);

	unsigned int lights = data.count - 1;

	if (lights > 0)
	{
		Array<BitPack> intersectResult(allocator);
		intersectResult.Resize(BitPack::CalculateRequired(lights));
		BitPack* intersected = intersectResult.GetData();
		Vec3f* positions = data.position + 1;
		float* radii = data.far + 1;

		Intersect::FrustumSphere(frustum, lights, positions, radii, intersected);

		for (unsigned int i = 1; i < data.count; ++i)
			if (data.type[i] != LightType::Directional && BitPack::Get(intersected, i - 1))
				result.PushBack(LightId{ i });
	}

	return result;
}