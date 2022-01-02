#include "Resources/UniformSerialization.hpp"

#include "Rendering/Uniform.hpp"
#include "Rendering/UniformData.hpp"

namespace
{

const size_t ItemSize = 4; // sizeof(float) or sizeof(int)


template <typename T>
void CreateVecValue(
	const kokko::UniformData& uniformData,
	const kokko::BufferUniform& uniform,
	rapidjson::Value& jsonValueOut,
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& jsonAllocator)
{
	const auto& value = uniformData.GetValue<T>(uniform);
	jsonValueOut.SetArray();
	for (size_t i = 0; i < sizeof(T) / ItemSize; ++i)
		jsonValueOut.PushBack(value[i], jsonAllocator);
}

template <typename T>
void CreateVecArray(
	const kokko::UniformData& uniformData,
	const kokko::BufferUniform& uniform,
	rapidjson::Value& jsonValueOut,
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& jsonAllocator)
{
	unsigned int count = 0;
	auto values = uniformData.GetArray<T>(uniform, count);
	jsonValueOut.SetArray();
	for (unsigned int i = 0; i < count; ++i)
	{
		rapidjson::Value arrayItem(rapidjson::kArrayType);
		for (size_t j = 0; j < sizeof(T) / ItemSize; ++j)
			arrayItem.PushBack(values[i][j], jsonAllocator);
		jsonValueOut.PushBack(arrayItem, jsonAllocator);
	}
}

template <typename T>
void CreateArray(
	const kokko::UniformData& uniformData,
	const kokko::BufferUniform& uniform,
	rapidjson::Value& jsonValueOut,
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& jsonAllocator)
{
	unsigned int count = 0;
	auto values = uniformData.GetArray<T>(uniform, count);
	jsonValueOut.SetArray();
	for (unsigned int i = 0; i < count; ++i)
		jsonValueOut.PushBack(values[i], jsonAllocator);
}

} // Anonymous namespace

namespace kokko
{
void SerializeUniformToJson(
	const UniformData& uniformData,
	const BufferUniform& uniform,
	rapidjson::Value& jsonValueOut,
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& jsonAllocator)
{
	switch (uniform.type)
	{
	case UniformDataType::Mat4x4:
		CreateVecValue<Mat4x4f>(uniformData, uniform, jsonValueOut, jsonAllocator);
		break;

	case UniformDataType::Mat4x4Array:
		CreateVecArray<Mat4x4f>(uniformData, uniform, jsonValueOut, jsonAllocator);
		break;

	case UniformDataType::Mat3x3:
		CreateVecValue<Mat3x3f>(uniformData, uniform, jsonValueOut, jsonAllocator);
		break;

	case UniformDataType::Mat3x3Array:
		CreateVecArray<Mat3x3f>(uniformData, uniform, jsonValueOut, jsonAllocator);
		break;

	case UniformDataType::Vec4:
		CreateVecValue<Vec4f>(uniformData, uniform, jsonValueOut, jsonAllocator);
		break;

	case UniformDataType::Vec4Array:
		CreateVecArray<Vec4f>(uniformData, uniform, jsonValueOut, jsonAllocator);
		break;

	case UniformDataType::Vec3:
		CreateVecValue<Vec3f>(uniformData, uniform, jsonValueOut, jsonAllocator);
		break;

	case UniformDataType::Vec3Array:
		CreateVecArray<Vec3f>(uniformData, uniform, jsonValueOut, jsonAllocator);
		break;

	case UniformDataType::Vec2:
		CreateVecValue<Vec2f>(uniformData, uniform, jsonValueOut, jsonAllocator);
		break;

	case UniformDataType::Vec2Array:
		CreateVecArray<Vec2f>(uniformData, uniform, jsonValueOut, jsonAllocator);
		break;

	case UniformDataType::Float:
		jsonValueOut.SetFloat(uniformData.GetValue<float>(uniform));
		break;

	case UniformDataType::FloatArray:
		CreateArray<float>(uniformData, uniform, jsonValueOut, jsonAllocator);
		break;

	case UniformDataType::Int:
		jsonValueOut.SetInt(uniformData.GetValue<int>(uniform));
		break;

	case UniformDataType::IntArray:
		CreateArray<int>(uniformData, uniform, jsonValueOut, jsonAllocator);
		break;
	}
}

}
