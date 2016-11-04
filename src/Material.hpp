#pragma once

#include <cstdint>

#include "Shader.hpp"

class ResourceManager;

struct ShaderMaterialUniform : ShaderUniform
{
	unsigned short dataOffset;
};

struct Material
{
	unsigned int shaderId;

	unsigned int uniformCount = 0;
	ShaderMaterialUniform uniforms[Shader::MaxMaterialUniforms];

	unsigned int usedUniformData = 0;
	unsigned char* uniformData = nullptr;

	void SetShader(const Shader* shader);

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
			if (uniforms[i].nameHash == uniformNameHash)
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
