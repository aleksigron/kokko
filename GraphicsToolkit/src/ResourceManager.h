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
	Collection<Texture, TextureId, 32> textures;
	Collection<ShaderProgram, ShaderProgramId, 32> shaders;

	ResourceManager();
	~ResourceManager();
};