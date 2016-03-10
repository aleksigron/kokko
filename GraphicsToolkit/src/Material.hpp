#pragma once

#include <cstdint>

#include "ObjectId.hpp"
#include "ShaderProgram.hpp"

struct ShaderMaterialUniform : ShaderUniform
{
	unsigned short dataOffset;
};

struct Material
{
	ObjectId id;

	ObjectId shader;

	static const unsigned int MaxUniformCount = 8;
	unsigned int uniformCount = 0;
	ShaderMaterialUniform uniforms[MaxUniformCount];

	unsigned int usedUniformData = 0;
	unsigned char* uniformData = nullptr;

	void SetShader(const ShaderProgram& shader);

	template <typename T>
	void SetUniformValue(unsigned int uniformIndex, const T& value)
	{
		if (uniformIndex < uniformCount && uniformData != nullptr)
		{
			unsigned char* data = uniformData + uniforms[uniformIndex].dataOffset;

			T* uniform = reinterpret_cast<T*>(data);
			*uniform = value;
		}
	}
};