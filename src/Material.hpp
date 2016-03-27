#pragma once

#include <cstdint>

#include "ShaderProgram.hpp"

class ResourceManager;

struct ShaderMaterialUniform : ShaderUniform
{
	unsigned short dataOffset;
};

struct Material
{
	uint32_t nameHash;

	unsigned int shaderId;

	unsigned int uniformCount = 0;
	ShaderMaterialUniform uniforms[ShaderProgram::MaxMaterialUniforms];
	uint32_t uniformNameHashes[ShaderProgram::MaxMaterialUniforms];

	unsigned int usedUniformData = 0;
	unsigned char* uniformData = nullptr;

	void SetShader(const ShaderProgram* shader);

	template <typename T>
	void SetUniformValueByIndex(unsigned int uniformIndex, const T& value)
	{
		unsigned char* data = uniformData + uniforms[uniformIndex].dataOffset;

		T* uniform = reinterpret_cast<T*>(data);
		*uniform = value;
	}

	template <typename T>
	void SetUniformValueByHash(uint32_t uniformNameHash, const T& value)
	{
		for (unsigned i = 0; i < uniformCount; ++i)
		{
			if (uniformNameHashes[i] == uniformNameHash)
			{
				unsigned char* data = uniformData + uniforms[i].dataOffset;

				T* uniform = reinterpret_cast<T*>(data);
				*uniform = value;

				break;
			}
		}
	}

	bool LoadFromConfiguration(Buffer<char>& configuration, ResourceManager* res);
};
