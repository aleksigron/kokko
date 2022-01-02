#pragma once

#include <cassert>
#include <cstdint>

#include "Core/ArrayView.hpp"

#include "Math/Mat4x4.hpp"

#include "Rendering/Uniform.hpp"
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

	// Access properties and textures

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
	
	// Get property values

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

	// Set property values

	template <typename ValueType>
	void SetValue(const BufferUniform& uniform, const ValueType& value);

	template <typename ValueType>
	void SetValueArray(const BufferUniform& uniform, unsigned int count, const ValueType* values);

	// Set texture values

	void SetTexture(TextureUniform& uniform, TextureId textureId, unsigned int textureObjectId);

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
