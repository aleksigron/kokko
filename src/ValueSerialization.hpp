#pragma once

#include "rapidjson/document.h"

#include "Vec3.hpp"

namespace ValueSerialization
{
	Vec3f Deserialize_Vec3f(const rapidjson::Value& value)
	{
		Vec3f result;

		assert(value.IsArray());
		assert(value.Size() >= 3);
		assert(value[0].IsNumber());
		assert(value[1].IsNumber());
		assert(value[2].IsNumber());

		result.x = value[0].GetFloat();
		result.y = value[1].GetFloat();
		result.z = value[2].GetFloat();

		return result;
	}
}
