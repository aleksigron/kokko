#pragma once

#include <cassert>

#include "rapidjson/document.h"

#include "Vec2.hpp"
#include "Vec3.hpp"
#include "Vec4.hpp"

#include "Mat2x2.hpp"
#include "Mat3x3.hpp"
#include "Mat4x4.hpp"

namespace ValueSerialization
{
	int Deserialize_Int(const rapidjson::Value& value)
	{
		assert(value.IsNumber());

		return value.GetInt();
	}

	float Deserialize_Float(const rapidjson::Value& value)
	{
		assert(value.IsNumber());

		return value.GetFloat();
	}

	Vec2f Deserialize_Vec2f(const rapidjson::Value& value)
	{
		Vec2f result;

		assert(value.IsArray());
		assert(value.Size() == 2);

		for (unsigned i = 0; i < 2; ++i)
		{
			assert(value[i].IsNumber());
			result[i] = value[i].GetFloat();
		}

		return result;
	}

	Vec3f Deserialize_Vec3f(const rapidjson::Value& value)
	{
		Vec3f result;

		assert(value.IsArray());
		assert(value.Size() == 3);

		for (unsigned i = 0; i < 3; ++i)
		{
			assert(value[i].IsNumber());
			result[i] = value[i].GetFloat();
		}

		return result;
	}

	Vec4f Deserialize_Vec4f(const rapidjson::Value& value)
	{
		Vec4f result;

		assert(value.IsArray());
		assert(value.Size() == 4);

		for (unsigned i = 0; i < 4; ++i)
		{
			assert(value[i].IsNumber());
			result[i] = value[i].GetFloat();
		}

		return result;
	}

	Mat2x2f Deserialize_Mat2x2f(const rapidjson::Value& value)
	{
		Mat2x2f result;

		assert(value.IsArray());
		assert(value.Size() == 4);

		for (unsigned i = 0; i < 4; ++i)
		{
			assert(value[i].IsNumber());
			result[i] = value[i].GetFloat();
		}

		return result;
	}

	Mat3x3f Deserialize_Mat3x3f(const rapidjson::Value& value)
	{
		Mat3x3f result;

		assert(value.IsArray());
		assert(value.Size() == 9);

		for (unsigned i = 0; i < 9; ++i)
		{
			assert(value[i].IsNumber());
			result[i] = value[i].GetFloat();
		}

		return result;
	}

	Mat4x4f Deserialize_Mat4x4f(const rapidjson::Value& value)
	{
		Mat4x4f result;

		assert(value.IsArray());
		assert(value.Size() == 16);

		for (unsigned i = 0; i < 16; ++i)
		{
			assert(value[i].IsNumber());
			result[i] = value[i].GetFloat();
		}

		return result;
	}
}
