#pragma once

#include <cstdint>
#include <cstddef>

#include "Collection.h"
#include "ShaderProgram.h"

class ResourceManager
{
private:

public:
	Collection<ShaderProgram, ShaderProgramId, 128> shaders;

	ResourceManager();
	~ResourceManager();
};