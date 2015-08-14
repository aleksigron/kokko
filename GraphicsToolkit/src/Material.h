#pragma once

#include <cstdint>

#include "ObjectId.h"
#include "ShaderProgram.h"

struct ShaderMaterialUniform : ShaderUniform
{
	unsigned short dataOffset;
};

struct Material
{
	ObjectId id;

	ObjectId shader;

	int mvpUniformLocation = -1;

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

inline bool operator < (const Material& lhs, const Material& rhs)
{
	return lhs.shader < rhs.shader;
}