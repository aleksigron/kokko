#pragma once

#include "Rendering/Shader.hpp"

struct MaterialUniform : ShaderUniform
{
	unsigned int dataOffset;
};

struct MaterialId
{
	unsigned int i;

	bool IsNull() const { return i == 0; }
};
