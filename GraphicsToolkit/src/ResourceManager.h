#pragma once

#include <cstdint>
#include <cstddef>

#include "Collection.h"
#include "Texture.h"
#include "ShaderProgram.h"

class ResourceManager
{
private:

public:
	Collection<Texture, 32> textures;
	Collection<ShaderProgram, 32> shaders;

	ResourceManager();
	~ResourceManager();
};