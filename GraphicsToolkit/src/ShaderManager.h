#pragma once

#include <cstdint>
#include <cstddef>

#include "Collection.h"
#include "ShaderProgram.h"

class ShaderManager
{
private:


public:
	Collection<ShaderProgram, ShaderProgramId, 128> shaders;

	ShaderManager();
	~ShaderManager();
};