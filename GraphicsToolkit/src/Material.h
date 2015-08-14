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
	static const unsigned MaxUniformCount = 8;

	ObjectId id;

	ObjectId shader;

	int mvpUniformLocation;

	unsigned int uniformCount;
	ShaderMaterialUniform uniforms[MaxUniformCount];

	unsigned int usedUniformData;
	unsigned char* uniformData;

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