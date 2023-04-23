#include "Resources/UniformSerialization.hpp"

#include "Rendering/Uniform.hpp"
#include "Rendering/UniformData.hpp"

namespace
{

const size_t ItemSize = 4; // sizeof(float) or sizeof(int)

// Serialization helpers

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

// Deserialization helpers


template <typename T>
T ReadVecValue(const rapidjson::Value& value)
{
	T result;

	assert(value.IsArray());
	assert(value.Size() == sizeof(T) / ItemSize);

	for (unsigned int i = 0, count = value.Size(); i < count; ++i)
	{
		assert(value[i].IsNumber());
		result[i] = value[i].GetFloat();
	}

	return result;
}

template <typename T>
void DeserializeVecValue(
	kokko::UniformData& uniformData,
	const kokko::BufferUniform& uniform,
	const rapidjson::Value* value)
{
	T result{};

	if (value != nullptr)
		result = ReadVecValue<T>(*value);

	uniformData.SetValue(uniform, result);
}

template <typename T>
void DeserializeVecArray(
	kokko::UniformData& uniformData,
	const kokko::BufferUniform& uniform,
	const rapidjson::Value* jsonValue,
	kokko::Array<unsigned char>& cacheBuffer)
{
	T* buffer = nullptr;
	unsigned int valueCount = 0;

	if (jsonValue != nullptr)
	{
		assert(jsonValue->IsArray());

		valueCount = jsonValue->Size();

		if (valueCount > uniform.arraySize)
			valueCount = uniform.arraySize;

		cacheBuffer.Resize(valueCount * sizeof(T));
		buffer = reinterpret_cast<T*>(cacheBuffer.GetData());

		for (unsigned int i = 0; i < valueCount; ++i)
			buffer[i] = ReadVecValue<T>((*jsonValue)[i]);
	}

	uniformData.SetValueArray(uniform, valueCount, buffer);
}

template <typename T>
void DeserializeValue(
	kokko::UniformData& uniformData,
	const kokko::BufferUniform& uniform,
	const rapidjson::Value* value)
{
	T result{};

	if (value != nullptr)
	{
		assert(value->IsNumber());
		result = value->Get<T>();
	}

	uniformData.SetValue(uniform, result);
}

template <typename T>
void DeserializeArray(
	kokko::UniformData& uniformData,
	const kokko::BufferUniform& uniform,
	const rapidjson::Value* jsonValue,
	kokko::Array<unsigned char>& cacheBuffer)
{
	T* buffer = nullptr;
	unsigned int valueCount = 0;

	if (jsonValue != nullptr)
	{
		assert(jsonValue->IsArray());

		valueCount = jsonValue->Size();

		if (valueCount > uniform.arraySize)
			valueCount = uniform.arraySize;

		cacheBuffer.Resize(valueCount * sizeof(T));
		buffer = reinterpret_cast<T*>(cacheBuffer.GetData());

		for (unsigned int i = 0; i < valueCount; ++i)
		{
			assert(jsonValue[i].IsNumber());
			buffer[i] = jsonValue[i].Get<T>();
		}
	}

	uniformData.SetValueArray(uniform, valueCount, buffer);
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

    default:
        break;
	}
}

void DeserializeUniformFromJson(
	kokko::UniformData& uniformData,
	const kokko::BufferUniform& uniform,
	const rapidjson::Value* jsonValue,
	Array<unsigned char>& cacheBuffer)
{
	KOKKO_PROFILE_FUNCTION();

	using ValueItr = rapidjson::Value::ConstValueIterator;

	switch (uniform.type)
	{
	case kokko::UniformDataType::Mat4x4:
		DeserializeVecValue<Mat4x4f>(uniformData, uniform, jsonValue);
		break;

	case kokko::UniformDataType::Mat4x4Array:
		DeserializeVecArray<Mat4x4f>(uniformData, uniform, jsonValue, cacheBuffer);
		break;

	case kokko::UniformDataType::Mat3x3:
		DeserializeVecValue<Mat3x3f>(uniformData, uniform, jsonValue);
		break;

	case kokko::UniformDataType::Mat3x3Array:
		DeserializeVecArray<Mat3x3f>(uniformData, uniform, jsonValue, cacheBuffer);
		break;

	case kokko::UniformDataType::Vec4:
		DeserializeVecValue<Vec4f>(uniformData, uniform, jsonValue);
		break;

	case kokko::UniformDataType::Vec4Array:
		DeserializeVecArray<Vec4f>(uniformData, uniform, jsonValue, cacheBuffer);
		break;

	case kokko::UniformDataType::Vec3:
		DeserializeVecValue<Vec3f>(uniformData, uniform, jsonValue);
		break;

	case kokko::UniformDataType::Vec3Array:
		DeserializeVecArray<Vec3f>(uniformData, uniform, jsonValue, cacheBuffer);
		break;

	case kokko::UniformDataType::Vec2:
		DeserializeVecValue<Vec2f>(uniformData, uniform, jsonValue);
		break;

	case kokko::UniformDataType::Vec2Array:
		DeserializeVecArray<Vec2f>(uniformData, uniform, jsonValue, cacheBuffer);
		break;

	case kokko::UniformDataType::Float:
		DeserializeValue<float>(uniformData, uniform, jsonValue);
		break;

	case kokko::UniformDataType::FloatArray:
		DeserializeArray<float>(uniformData, uniform, jsonValue, cacheBuffer);
		break;

	case kokko::UniformDataType::Int:
		DeserializeValue<int>(uniformData, uniform, jsonValue);
		break;

	case kokko::UniformDataType::IntArray:
		DeserializeArray<int>(uniformData, uniform, jsonValue, cacheBuffer);
		break;

	default:
		break;
	}
}

}
