#include "Rendering/UniformData.hpp"

#include <cassert>

#include "Core/StringRef.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/Uniform.hpp"
#include "Rendering/UniformBuffer.hpp"

namespace kokko
{

UniformData::UniformData(Allocator* allocator) :
	allocator(allocator),
	buffer(nullptr),
	uniformDataBuffer(nullptr)
{
}

void UniformData::Initialize(const UniformList& from)
{
	if (buffer != nullptr) // Release old uniform data
	{
		allocator->Deallocate(buffer);
		buffer = nullptr;
	}

	definitions.uniformDataSize = from.uniformDataSize;
	definitions.uniformBufferSize = from.uniformBufferSize;
	definitions.bufferUniformCount = from.bufferUniformCount;
	definitions.textureUniformCount = from.textureUniformCount;
	definitions.bufferUniforms = nullptr;
	definitions.textureUniforms = nullptr;

	size_t uboSize = from.uniformDataSize;

	if (definitions.bufferUniformCount > 0 ||
		definitions.textureUniformCount > 0)
	{
		size_t bufferSize = definitions.bufferUniformCount * sizeof(kokko::BufferUniform) +
			definitions.textureUniformCount * sizeof(kokko::TextureUniform) +
			definitions.uniformDataSize;

		buffer = allocator->Allocate(bufferSize);

		auto bufferBuf = static_cast<kokko::BufferUniform*>(buffer);
		auto textureBuf = reinterpret_cast<kokko::TextureUniform*>(bufferBuf + definitions.bufferUniformCount);
		auto dataBuf = reinterpret_cast<uint8_t*>(textureBuf + definitions.textureUniformCount);

		definitions.bufferUniforms = definitions.bufferUniformCount > 0 ? bufferBuf : nullptr;
		definitions.textureUniforms = definitions.textureUniformCount > 0 ? textureBuf : nullptr;
		uniformDataBuffer = definitions.bufferUniformCount > 0 ? dataBuf : nullptr;

		for (size_t i = 0, count = definitions.bufferUniformCount; i < count; ++i)
			definitions.bufferUniforms[i] = from.bufferUniforms[i];

		for (size_t i = 0, count = definitions.textureUniformCount; i < count; ++i)
			definitions.textureUniforms[i] = from.textureUniforms[i];
	}
	else
	{
		buffer = nullptr;
		uniformDataBuffer = nullptr;
	}
}

void UniformData::Release()
{
	if (buffer != nullptr)
	{
		allocator->Deallocate(buffer);
		buffer = nullptr;
	}
}

unsigned int UniformData::GetUniformBufferSize() const
{
	return definitions.uniformBufferSize;
}

ArrayView<BufferUniform> UniformData::GetBufferUniforms()
{
	return ArrayView<BufferUniform>(definitions.bufferUniforms, definitions.bufferUniformCount);
}

ArrayView<const BufferUniform> UniformData::GetBufferUniforms() const
{
	return ArrayView<const BufferUniform>(definitions.bufferUniforms, definitions.bufferUniformCount);
}

BufferUniform* UniformData::FindBufferUniformByName(StringRef name)
{
	for (size_t i = 0, count = definitions.bufferUniformCount; i < count; ++i)
		if (definitions.bufferUniforms[i].name.ValueEquals(name))
			return &definitions.bufferUniforms[i];

	return nullptr;
}

const BufferUniform* UniformData::FindBufferUniformByName(StringRef name) const
{
	for (size_t i = 0, count = definitions.bufferUniformCount; i < count; ++i)
		if (definitions.bufferUniforms[i].name.ValueEquals(name))
			return &definitions.bufferUniforms[i];

	return nullptr;
}

BufferUniform* UniformData::FindBufferUniformByNameHash(uint32_t nameHash)
{
	for (size_t i = 0, count = definitions.bufferUniformCount; i < count; ++i)
		if (definitions.bufferUniforms[i].nameHash == nameHash)
			return &definitions.bufferUniforms[i];

	return nullptr;
}

const BufferUniform* UniformData::FindBufferUniformByNameHash(uint32_t nameHash) const
{
	for (size_t i = 0, count = definitions.bufferUniformCount; i < count; ++i)
		if (definitions.bufferUniforms[i].nameHash == nameHash)
			return &definitions.bufferUniforms[i];

	return nullptr;
}

ArrayView<TextureUniform> UniformData::GetTextureUniforms()
{
	return ArrayView<TextureUniform>(definitions.textureUniforms, definitions.textureUniformCount);
}

ArrayView<const TextureUniform> UniformData::GetTextureUniforms() const
{
	return ArrayView<const TextureUniform>(definitions.textureUniforms, definitions.textureUniformCount);
}

TextureUniform* UniformData::FindTextureUniformByName(StringRef name)
{
	return definitions.FindTextureUniformByName(name);
}

const TextureUniform* UniformData::FindTextureUniformByName(StringRef name) const
{
	return definitions.FindTextureUniformByName(name);
}

TextureUniform* UniformData::FindTextureUniformByNameHash(uint32_t nameHash)
{
	return definitions.FindTextureUniformByNameHash(nameHash);
}

const TextureUniform* UniformData::FindTextureUniformByNameHash(uint32_t nameHash) const
{
	return definitions.FindTextureUniformByNameHash(nameHash);
}

void UniformData::SetValueInt(const BufferUniform& uniform, int value) const
{
	*reinterpret_cast<int*>(uniformDataBuffer + uniform.dataOffset) = value;
}

void UniformData::SetArrayInt(const BufferUniform& uniform, const int* values, unsigned int count) const
{
	assert(count <= uniform.arraySize);

	*reinterpret_cast<unsigned int*>(uniformDataBuffer + uniform.dataOffset) = count;

	int* dataStart = reinterpret_cast<int*>(uniformDataBuffer + uniform.dataOffset + sizeof(unsigned int));
	for (unsigned int i = 0; i < count; ++i)
		dataStart[i] = values[i];
}

void UniformData::SetValueFloat(const BufferUniform& uniform, float value) const
{
	*reinterpret_cast<float*>(uniformDataBuffer + uniform.dataOffset) = value;
}

void UniformData::SetArrayFloat(const BufferUniform& uniform, const float* values, unsigned int count) const
{
	assert(count <= uniform.arraySize);

	*reinterpret_cast<unsigned int*>(uniformDataBuffer + uniform.dataOffset) = count;

	float* dataStart = reinterpret_cast<float*>(uniformDataBuffer + uniform.dataOffset + sizeof(unsigned int));
	for (unsigned int i = 0; i < count; ++i)
		dataStart[i] = values[i];
}

void UniformData::SetValueVec2f(const BufferUniform& uniform, const Vec2f& value) const
{
	SetValueFloatVec(uniform, 2, value.ValuePointer());
}

void UniformData::SetArrayVec2f(const BufferUniform& uniform, const Vec2f* values, unsigned int count) const
{
	assert(count <= uniform.arraySize);

	*reinterpret_cast<unsigned int*>(uniformDataBuffer + uniform.dataOffset) = count;

	float* dataStart = reinterpret_cast<float*>(uniformDataBuffer + uniform.dataOffset + sizeof(unsigned int));
	for (unsigned int i = 0; i < count; ++i)
	{
		dataStart[i * 2 + 0] = values[i].x;
		dataStart[i * 2 + 1] = values[i].y;
	}
}

void UniformData::SetValueVec3f(const BufferUniform& uniform, const Vec3f& value) const
{
	SetValueFloatVec(uniform, 3, value.ValuePointer());
}

void UniformData::SetArrayVec3f(const BufferUniform& uniform, const Vec3f* values, unsigned int count) const
{
	assert(count <= uniform.arraySize);

	*reinterpret_cast<unsigned int*>(uniformDataBuffer + uniform.dataOffset) = count;

	float* dataStart = reinterpret_cast<float*>(uniformDataBuffer + uniform.dataOffset + sizeof(unsigned int));
	for (unsigned int i = 0; i < count; ++i)
	{
		dataStart[i * 3 + 0] = values[i].x;
		dataStart[i * 3 + 1] = values[i].y;
		dataStart[i * 3 + 2] = values[i].z;
	}
}

void UniformData::SetValueVec4f(const BufferUniform& uniform, const Vec4f& value) const
{
	SetValueFloatVec(uniform, 4, value.ValuePointer());
}

void UniformData::SetArrayVec4f(const BufferUniform& uniform, const Vec4f* values, unsigned int count) const
{
	assert(count <= uniform.arraySize);

	*reinterpret_cast<unsigned int*>(uniformDataBuffer + uniform.dataOffset) = count;

	float* dataStart = reinterpret_cast<float*>(uniformDataBuffer + uniform.dataOffset + sizeof(unsigned int));
	for (unsigned int i = 0; i < count; ++i)
	{
		dataStart[i * 4 + 0] = values[i].x;
		dataStart[i * 4 + 1] = values[i].y;
		dataStart[i * 4 + 2] = values[i].z;
		dataStart[i * 4 + 3] = values[i].w;
	}
}

void UniformData::SetValueMat3x3f(const BufferUniform& uniform, const Mat3x3f& value) const
{
	SetValueFloatVec(uniform, 9, value.ValuePointer());
}

void UniformData::SetArrayMat3x3f(const BufferUniform& uniform, const Mat3x3f* values, unsigned int count) const
{
	assert(count <= uniform.arraySize);

	*reinterpret_cast<unsigned int*>(uniformDataBuffer + uniform.dataOffset) = count;

	float* dataStart = reinterpret_cast<float*>(uniformDataBuffer + uniform.dataOffset + sizeof(unsigned int));
	for (unsigned int i = 0; i < count; ++i)
		for (unsigned int j = 0; j < 9; ++j)
			dataStart[i * 9 + j] = values[i][j];
}

void UniformData::SetValueMat4x4f(const BufferUniform& uniform, const Mat4x4f& value) const
{
	SetValueFloatVec(uniform, 16, value.ValuePointer());
}

void UniformData::SetArrayMat4x4f(const BufferUniform& uniform, const Mat4x4f* values, unsigned int count) const
{
	assert(count <= uniform.arraySize);

	*reinterpret_cast<unsigned int*>(uniformDataBuffer + uniform.dataOffset) = count;

	float* dataStart = reinterpret_cast<float*>(uniformDataBuffer + uniform.dataOffset + sizeof(unsigned int));
	for (unsigned int i = 0; i < count; ++i)
		for (unsigned int j = 0; j < 16; ++j)
			dataStart[i * 16 + j] = values[i][j];
}

void UniformData::WriteToUniformBuffer(uint8_t* uniformBuffer) const
{
	for (size_t i = 0, count = definitions.bufferUniformCount; i < count; ++i)
	{
		const BufferUniform& uniform = definitions.bufferUniforms[i];

		switch (uniform.type)
		{
		case UniformDataType::Tex2D:
		case UniformDataType::TexCube:
			break;

		case UniformDataType::Mat4x4:
		{
			const Mat4x4f& val = GetValue<Mat4x4f>(uniform);
			UniformBuffer::SetScalarMat4x4f(uniformBuffer, uniform.bufferObjectOffset, val);
			break;
		}
		case UniformDataType::Mat4x4Array:
		{
			unsigned int countOut;
			const Mat4x4f* values = GetArray<Mat4x4f>(uniform, countOut);
			UniformBuffer::SetArrayMat4x4fMany(uniformBuffer, uniform.bufferObjectOffset, countOut, values);
			break;
		}
		case UniformDataType::Mat3x3:
		{
			const Mat3x3f& val = GetValue<Mat3x3f>(uniform);
			UniformBuffer::SetScalarMat4x4f(uniformBuffer, uniform.bufferObjectOffset, val);
			break;
		}
		case UniformDataType::Mat3x3Array:
		{
			unsigned int countOut;
			const Mat3x3f* values = GetArray<Mat3x3f>(uniform, countOut);
			UniformBuffer::SetArrayMat3x3fMany(uniformBuffer, uniform.bufferObjectOffset, countOut, values);
			break;
		}
		case UniformDataType::Vec4:
		{
			Vec4f* val = reinterpret_cast<Vec4f*>(uniformDataBuffer + uniform.dataOffset);
			UniformBuffer::SetScalarVec4f(uniformBuffer, uniform.bufferObjectOffset, *val);
			break;
		}
		case UniformDataType::Vec4Array:
		{
			unsigned int countOut;
			const Vec4f* values = GetArray<Vec4f>(uniform, countOut);
			UniformBuffer::SetArrayVec4fMany(uniformBuffer, uniform.bufferObjectOffset, countOut, values);
			break;
		}
		case UniformDataType::Vec3:
		{
			Vec3f* val = reinterpret_cast<Vec3f*>(uniformDataBuffer + uniform.dataOffset);
			UniformBuffer::SetScalarVec3f(uniformBuffer, uniform.bufferObjectOffset, *val);
			break;
		}
		case UniformDataType::Vec3Array:
		{
			unsigned int countOut;
			const Vec3f* values = GetArray<Vec3f>(uniform, countOut);
			UniformBuffer::SetArrayVec3fMany(uniformBuffer, uniform.bufferObjectOffset, countOut, values);
			break;
		}
		case UniformDataType::Vec2:
		{
			Vec2f* val = reinterpret_cast<Vec2f*>(uniformDataBuffer + uniform.dataOffset);
			UniformBuffer::SetScalarVec2f(uniformBuffer, uniform.bufferObjectOffset, *val);
			break;
		}
		case UniformDataType::Vec2Array:
		{
			unsigned int countOut;
			const Vec2f* values = GetArray<Vec2f>(uniform, countOut);
			UniformBuffer::SetArrayVec2fMany(uniformBuffer, uniform.bufferObjectOffset, countOut, values);
			break;
		}
		case UniformDataType::Float:
		{
			float* val = reinterpret_cast<float*>(uniformDataBuffer + uniform.dataOffset);
			UniformBuffer::SetScalarFloat(uniformBuffer, uniform.bufferObjectOffset, *val);
			break;
		}
		case UniformDataType::FloatArray:
		{
			unsigned int countOut;
			const float* values = GetArray<float>(uniform, countOut);
			UniformBuffer::SetArrayFloatMany(uniformBuffer, uniform.bufferObjectOffset, countOut, values);
			break;
		}
		case UniformDataType::Int:
		{
			int* val = reinterpret_cast<int*>(uniformDataBuffer + uniform.dataOffset);
			UniformBuffer::SetScalarInt(uniformBuffer, uniform.bufferObjectOffset, *val);
			break;
		}
		case UniformDataType::IntArray:
		{
			unsigned int countOut;
			const int* values = GetArray<int>(uniform, countOut);
			UniformBuffer::SetArrayIntMany(uniformBuffer, uniform.bufferObjectOffset, countOut, values);
			break;
		}

		default:
			break;
		}
	}
}

void UniformData::SetValueFloatVec(const BufferUniform& uniform, unsigned int count, const float* values) const
{
	for (unsigned int i = 0; i < count; ++i)
		*reinterpret_cast<float*>(uniformDataBuffer + uniform.dataOffset + i * 4) = values[i];
}

}
