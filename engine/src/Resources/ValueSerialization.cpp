#include "Resources/ValueSerialization.hpp"

#include <cassert>

int ValueSerialization::Deserialize_Int(const rapidjson::Value& value)
{
	assert(value.IsNumber());

	return value.GetInt();
}

float ValueSerialization::Deserialize_Float(const rapidjson::Value& value)
{
	assert(value.IsNumber());

	return value.GetFloat();
}

Vec2f ValueSerialization::Deserialize_Vec2f(const rapidjson::Value& value)
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

Vec3f ValueSerialization::Deserialize_Vec3f(const rapidjson::Value& value)
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

Vec4f ValueSerialization::Deserialize_Vec4f(const rapidjson::Value& value)
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

Mat2x2f ValueSerialization::Deserialize_Mat2x2f(const rapidjson::Value& value)
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

Mat3x3f ValueSerialization::Deserialize_Mat3x3f(const rapidjson::Value& value)
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

Mat4x4f ValueSerialization::Deserialize_Mat4x4f(const rapidjson::Value& value)
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

Color ValueSerialization::Deserialize_Color(const rapidjson::Value& value)
{
	Color result;

	assert(value.IsArray());
	assert(value.Size() >= 3);
	assert(value[0].IsNumber());
	assert(value[1].IsNumber());
	assert(value[2].IsNumber());

	result.r = value[0].GetFloat();
	result.g = value[1].GetFloat();
	result.b = value[2].GetFloat();

	if (value.Size() >= 4)
	{
		assert(value[3].IsNumber());
		result.a = value[3].GetFloat();
	}
	else
		result.a = 1.0f;

	return result;
}