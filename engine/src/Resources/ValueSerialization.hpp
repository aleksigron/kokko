#pragma once

#include "rapidjson/document.h"

#include "Math/Vec2.hpp"
#include "Math/Vec3.hpp"
#include "Math/Vec4.hpp"

#include "Math/Mat2x2.hpp"
#include "Math/Mat3x3.hpp"
#include "Math/Mat4x4.hpp"

#include "Core/Color.hpp"

namespace ValueSerialization
{
	int Deserialize_Int(const rapidjson::Value& value);
	float Deserialize_Float(const rapidjson::Value& value);
	Vec2f Deserialize_Vec2f(const rapidjson::Value& value);
	Vec3f Deserialize_Vec3f(const rapidjson::Value& value);
	Vec4f Deserialize_Vec4f(const rapidjson::Value& value);
	Mat2x2f Deserialize_Mat2x2f(const rapidjson::Value& value);
	Mat3x3f Deserialize_Mat3x3f(const rapidjson::Value& value);
	Mat4x4f Deserialize_Mat4x4f(const rapidjson::Value& value);
	Color Deserialize_Color(const rapidjson::Value& value);
}
