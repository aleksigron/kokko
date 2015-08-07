#pragma once

#include "Collection.h"
#include "Texture.h"
#include "ShaderProgram.h"
#include "Material.h"

class ResourceManager
{
public:
	Collection<Texture, 32> textures;
	Collection<ShaderProgram, 32> shaders;
	Collection<Material, 32> materials;

	ResourceManager();
	~ResourceManager();
};