#pragma once

#include <cstddef>
#include <cstdint>

#include "Core/StringView.hpp"

namespace kokko
{

struct BufferUniform;
struct TextureUniform;

struct UniformList
{
	size_t uniformDataSize = 0; // CPU side
	unsigned int uniformBufferSize = 0; // GPU side

	size_t bufferUniformCount = 0;
	size_t textureUniformCount = 0;
	BufferUniform* bufferUniforms = nullptr;
	TextureUniform* textureUniforms = nullptr;

	TextureUniform* FindTextureUniformByName(ConstStringView name);
	const TextureUniform* FindTextureUniformByName(ConstStringView name) const;

	TextureUniform* FindTextureUniformByNameHash(uint32_t nameHash);
	const TextureUniform* FindTextureUniformByNameHash(uint32_t nameHash) const;
};

}
