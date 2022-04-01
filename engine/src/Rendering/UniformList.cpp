#include "Rendering/UniformList.hpp"

#include "Core/StringView.hpp"

#include "Rendering/Uniform.hpp"

namespace kokko
{

TextureUniform* UniformList::FindTextureUniformByName(ConstStringView name)
{
	for (size_t i = 0, count = textureUniformCount; i < count; ++i)
		if (textureUniforms[i].name.ValueEquals(name))
			return &textureUniforms[i];

	return nullptr;
}

const TextureUniform* UniformList::FindTextureUniformByName(ConstStringView name) const
{
	for (size_t i = 0, count = textureUniformCount; i < count; ++i)
		if (textureUniforms[i].name.ValueEquals(name))
			return &textureUniforms[i];

	return nullptr;
}

TextureUniform* UniformList::FindTextureUniformByNameHash(uint32_t nameHash)
{
	for (size_t i = 0, count = textureUniformCount; i < count; ++i)
		if (textureUniforms[i].nameHash == nameHash)
			return &textureUniforms[i];

	return nullptr;
}

const TextureUniform* UniformList::FindTextureUniformByNameHash(uint32_t nameHash) const
{
	for (size_t i = 0, count = textureUniformCount; i < count; ++i)
		if (textureUniforms[i].nameHash == nameHash)
			return &textureUniforms[i];

	return nullptr;
}

}
