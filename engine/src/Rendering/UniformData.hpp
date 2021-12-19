#pragma once

#include <cstdint>

#include "Core/ArrayView.hpp"

#include "Math/Mat4x4.hpp"

#include "Rendering/UniformList.hpp"

class Allocator;

namespace kokko
{
struct BufferUniform;
struct TextureUniform;

class UniformData
{
public:
	UniformData(Allocator* allocator);

	void Initialize(const UniformList& fromUniformList);
	void Release();

	unsigned int GetUniformBufferSize() const;

	ArrayView<BufferUniform> GetBufferUniforms();
	ArrayView<const BufferUniform> GetBufferUniforms() const;

	BufferUniform* FindBufferUniformByName(StringRef name);
	const BufferUniform* FindBufferUniformByName(StringRef name) const;

	BufferUniform* FindBufferUniformByNameHash(uint32_t nameHash);
	const BufferUniform* FindBufferUniformByNameHash(uint32_t nameHash) const;

	ArrayView<TextureUniform> GetTextureUniforms();
	ArrayView<const TextureUniform> GetTextureUniforms() const;

	TextureUniform* FindTextureUniformByName(StringRef name);
	const TextureUniform* FindTextureUniformByName(StringRef name) const;

	TextureUniform* FindTextureUniformByNameHash(uint32_t nameHash);
	const TextureUniform* FindTextureUniformByNameHash(uint32_t nameHash) const;

	template <typename ValueType>
	const ValueType* GetArray(const BufferUniform& uniform, unsigned int& countOut) const
	{
		countOut = *reinterpret_cast<unsigned int*>(uniformDataBuffer + uniform.dataOffset);
		return reinterpret_cast<const ValueType*>(uniformDataBuffer + uniform.dataOffset + sizeof(unsigned int));
	}

	template <typename ValueType>
	const ValueType& GetValue(const BufferUniform& uniform) const
	{
		return *reinterpret_cast<const ValueType*>(uniformDataBuffer + uniform.dataOffset);
	}

	void SetValueInt(const BufferUniform& uniform, int value) const;
	void SetArrayInt(const BufferUniform& uniform, const int* values, unsigned int count) const;

	void SetValueFloat(const BufferUniform& uniform, float value) const;
	void SetArrayFloat(const BufferUniform& uniform, const float* values, unsigned int count) const;

	void SetValueVec2f(const BufferUniform& uniform, const Vec2f& value) const;
	void SetArrayVec2f(const BufferUniform& uniform, const Vec2f* values, unsigned int count) const;

	void SetValueVec3f(const BufferUniform& uniform, const Vec3f& value) const;
	void SetArrayVec3f(const BufferUniform& uniform, const Vec3f* values, unsigned int count) const;

	void SetValueVec4f(const BufferUniform& uniform, const Vec4f& value) const;
	void SetArrayVec4f(const BufferUniform& uniform, const Vec4f* values, unsigned int count) const;

	void SetValueMat3x3f(const BufferUniform& uniform, const Mat3x3f& value) const;
	void SetArrayMat3x3f(const BufferUniform& uniform, const Mat3x3f* values, unsigned int count) const;

	void SetValueMat4x4f(const BufferUniform& uniform, const Mat4x4f& value) const;
	void SetArrayMat4x4f(const BufferUniform& uniform, const Mat4x4f* values, unsigned int count) const;

	void WriteToUniformBuffer(uint8_t* uniformBuffer) const;

private:
	void SetValueFloatVec(const BufferUniform& uniform, unsigned int count, const float* values) const;

private:
	Allocator* allocator;

	void* buffer;
	uint8_t* uniformDataBuffer;

	UniformList definitions;
};

}
