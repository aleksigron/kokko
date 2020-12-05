#include "Rendering/Uniform.hpp"

#include <cstring>
#include <cassert>

#include "Rendering/UniformBuffer.hpp"

UniformTypeInfo::UniformTypeInfo(unsigned int size, unsigned int alignment,
	const char* typeName, bool isArray, bool isTexture) :
	size(size),
	alignment(alignment),
	typeNameLength(0),
	typeName(typeName),
	isArray(isArray),
	isTexture(isTexture)
{
	typeNameLength = std::strlen(typeName);
}

// Must match order with UniformDataType
const UniformTypeInfo UniformTypeInfo::Types[] = {
	UniformTypeInfo(4, 4, "", false, true), // Texture2D
	UniformTypeInfo(4, 4, "", false, true), // TextureCube
	UniformTypeInfo(64, 16, "mat4x4", false, false),
	UniformTypeInfo(64, 16, "mat4x4", true, false),
	UniformTypeInfo(48, 16, "mat3x3", false, false),
	UniformTypeInfo(48, 16, "mat3x3", true, false),
	UniformTypeInfo(16, 16, "vec4", false, false),
	UniformTypeInfo(16, 16, "vec4", true, false),
	UniformTypeInfo(16, 16, "vec3", false, false),
	UniformTypeInfo(16, 16, "vec3", true, false),
	UniformTypeInfo(8, 8, "vec2", false, false),
	UniformTypeInfo(8, 16, "vec2", true, false),
	UniformTypeInfo(4, 4, "float", false, false),
	UniformTypeInfo(4, 16, "float", true, false),
	UniformTypeInfo(4, 4, "int", false, false),
	UniformTypeInfo(4, 16, "int", true, false)
};

void BufferUniform::SetValueInt(unsigned char* dataBuffer, int value) const
{
	*reinterpret_cast<int*>(dataBuffer + dataOffset) = value;
}

void BufferUniform::SetArrayInt(unsigned char* dataBuffer, const int* values, unsigned int count) const
{
	assert(count <= arraySize);

	*reinterpret_cast<unsigned int*>(dataBuffer + dataOffset) = count;

	int* dataStart = reinterpret_cast<int*>(dataBuffer + dataOffset + sizeof(unsigned int));
	for (unsigned int i = 0; i < count; ++i)
		dataStart[i] = values[i];
}

void BufferUniform::SetValueFloat(unsigned char* dataBuffer, float value) const
{
	*reinterpret_cast<float*>(dataBuffer + dataOffset) = value;
}

void BufferUniform::SetArrayFloat(unsigned char* dataBuffer, const float* values, unsigned int count) const
{
	assert(count <= arraySize);

	*reinterpret_cast<unsigned int*>(dataBuffer + dataOffset) = count;

	float* dataStart = reinterpret_cast<float*>(dataBuffer + dataOffset + sizeof(unsigned int));
	for (unsigned int i = 0; i < count; ++i)
		dataStart[i] = values[i];
}

void BufferUniform::SetValueFloatVec(unsigned char* dataBuffer, unsigned int count, const float* values) const
{
	for (unsigned int i = 0; i < count; ++i)
		*reinterpret_cast<float*>(dataBuffer + dataOffset + i * 4) = values[i];
}

void BufferUniform::SetValueVec2f(unsigned char* dataBuffer, const Vec2f& value) const
{
	SetValueFloatVec(dataBuffer, 2, value.ValuePointer());
}

void BufferUniform::SetArrayVec2f(unsigned char* dataBuffer, const Vec2f* values, unsigned int count) const
{
	assert(count <= arraySize);

	*reinterpret_cast<unsigned int*>(dataBuffer + dataOffset) = count;

	float* dataStart = reinterpret_cast<float*>(dataBuffer + dataOffset + sizeof(unsigned int));
	for (unsigned int i = 0; i < count; ++i)
	{
		dataStart[i * 2 + 0] = values[i].x;
		dataStart[i * 2 + 1] = values[i].y;
	}
}

void BufferUniform::SetValueVec3f(unsigned char* dataBuffer, const Vec3f& value) const
{
	SetValueFloatVec(dataBuffer, 3, value.ValuePointer());
}

void BufferUniform::SetArrayVec3f(unsigned char* dataBuffer, const Vec3f* values, unsigned int count) const
{
	assert(count <= arraySize);

	*reinterpret_cast<unsigned int*>(dataBuffer + dataOffset) = count;

	float* dataStart = reinterpret_cast<float*>(dataBuffer + dataOffset + sizeof(unsigned int));
	for (unsigned int i = 0; i < count; ++i)
	{
		dataStart[i * 3 + 0] = values[i].x;
		dataStart[i * 3 + 1] = values[i].y;
		dataStart[i * 3 + 2] = values[i].z;
	}
}

void BufferUniform::SetValueVec4f(unsigned char* dataBuffer, const Vec4f& value) const
{
	SetValueFloatVec(dataBuffer, 4, value.ValuePointer());
}

void BufferUniform::SetArrayVec4f(unsigned char* dataBuffer, const Vec4f* values, unsigned int count) const
{
	assert(count <= arraySize);

	*reinterpret_cast<unsigned int*>(dataBuffer + dataOffset) = count;

	float* dataStart = reinterpret_cast<float*>(dataBuffer + dataOffset + sizeof(unsigned int));
	for (unsigned int i = 0; i < count; ++i)
	{
		dataStart[i * 4 + 0] = values[i].x;
		dataStart[i * 4 + 1] = values[i].y;
		dataStart[i * 4 + 2] = values[i].z;
		dataStart[i * 4 + 3] = values[i].w;
	}
}

void BufferUniform::SetValueMat3x3f(unsigned char* dataBuffer, const Mat3x3f& value) const
{
	SetValueFloatVec(dataBuffer, 9, value.ValuePointer());
}

void BufferUniform::SetArrayMat3x3f(unsigned char* dataBuffer, const Mat3x3f* values, unsigned int count) const
{
	assert(count <= arraySize);

	*reinterpret_cast<unsigned int*>(dataBuffer + dataOffset) = count;

	float* dataStart = reinterpret_cast<float*>(dataBuffer + dataOffset + sizeof(unsigned int));
	for (unsigned int i = 0; i < count; ++i)
		for (unsigned int j = 0; j < 9; ++j)
			dataStart[i * 9 + j] = values[i][j];
}

void BufferUniform::SetValueMat4x4f(unsigned char* dataBuffer, const Mat4x4f& value) const
{
	SetValueFloatVec(dataBuffer, 16, value.ValuePointer());
}

void BufferUniform::SetArrayMat4x4f(unsigned char* dataBuffer, const Mat4x4f* values, unsigned int count) const
{
	assert(count <= arraySize);

	*reinterpret_cast<unsigned int*>(dataBuffer + dataOffset) = count;

	float* dataStart = reinterpret_cast<float*>(dataBuffer + dataOffset + sizeof(unsigned int));
	for (unsigned int i = 0; i < count; ++i)
		for (unsigned int j = 0; j < 16; ++j)
			dataStart[i * 16 + j] = values[i][j];
}

void BufferUniform::UpdateToUniformBuffer(unsigned char* dataBuffer, unsigned char* uniformBuffer)
{
	switch (type)
	{
	case UniformDataType::Tex2D:
	case UniformDataType::TexCube:
		break;

	case UniformDataType::Mat4x4:
	{
		const Mat4x4f& val = GetValue<Mat4x4f>(dataBuffer);
		UniformBuffer::SetScalarMat4x4f(uniformBuffer, bufferObjectOffset, val);
		break;
	}
	case UniformDataType::Mat4x4Array:
	{
		unsigned int countOut;
		const Mat4x4f* values = GetArray<Mat4x4f>(dataBuffer, countOut);
		UniformBuffer::SetArrayMat4x4fMany(uniformBuffer, bufferObjectOffset, countOut, values);
		break;
	}
	case UniformDataType::Mat3x3:
	{
		const Mat3x3f& val = GetValue<Mat3x3f>(dataBuffer);
		UniformBuffer::SetScalarMat4x4f(uniformBuffer, bufferObjectOffset, val);
		break;
	}
	case UniformDataType::Mat3x3Array:
	{
		unsigned int countOut;
		const Mat3x3f* values = GetArray<Mat3x3f>(dataBuffer, countOut);
		UniformBuffer::SetArrayMat3x3fMany(uniformBuffer, bufferObjectOffset, countOut, values);
		break;
	}
	case UniformDataType::Vec4:
	{
		Vec4f* val = reinterpret_cast<Vec4f*>(dataBuffer + dataOffset);
		UniformBuffer::SetScalarVec4f(uniformBuffer, bufferObjectOffset, *val);
		break;
	}
	case UniformDataType::Vec4Array:
	{
		unsigned int countOut;
		const Vec4f* values = GetArray<Vec4f>(dataBuffer, countOut);
		UniformBuffer::SetArrayVec4fMany(uniformBuffer, bufferObjectOffset, countOut, values);
		break;
	}
	case UniformDataType::Vec3:
	{
		Vec3f* val = reinterpret_cast<Vec3f*>(dataBuffer + dataOffset);
		UniformBuffer::SetScalarVec3f(uniformBuffer, bufferObjectOffset, *val);
		break;
	}
	case UniformDataType::Vec3Array:
	{
		unsigned int countOut;
		const Vec3f* values = GetArray<Vec3f>(dataBuffer, countOut);
		UniformBuffer::SetArrayVec3fMany(uniformBuffer, bufferObjectOffset, countOut, values);
		break;
	}
	case UniformDataType::Vec2:
	{
		Vec2f* val = reinterpret_cast<Vec2f*>(dataBuffer + dataOffset);
		UniformBuffer::SetScalarVec2f(uniformBuffer, bufferObjectOffset, *val);
		break;
	}
	case UniformDataType::Vec2Array:
	{
		unsigned int countOut;
		const Vec2f* values = GetArray<Vec2f>(dataBuffer, countOut);
		UniformBuffer::SetArrayVec2fMany(uniformBuffer, bufferObjectOffset, countOut, values);
		break;
	}
	case UniformDataType::Float:
	{
		float* val = reinterpret_cast<float*>(dataBuffer + dataOffset);
		UniformBuffer::SetScalarFloat(uniformBuffer, bufferObjectOffset, *val);
		break;
	}
	case UniformDataType::FloatArray:
	{
		unsigned int countOut;
		const float* values = GetArray<float>(dataBuffer, countOut);
		UniformBuffer::SetArrayFloatMany(uniformBuffer, bufferObjectOffset, countOut, values);
		break;
	}
	case UniformDataType::Int:
	{
		int* val = reinterpret_cast<int*>(dataBuffer + dataOffset);
		UniformBuffer::SetScalarInt(uniformBuffer, bufferObjectOffset, *val);
		break;
	}
	case UniformDataType::IntArray:
	{
		unsigned int countOut;
		const int* values = GetArray<int>(dataBuffer, countOut);
		UniformBuffer::SetArrayIntMany(uniformBuffer, bufferObjectOffset, countOut, values);
		break;
	}

	default:
		break;
	}
}