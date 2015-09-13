#pragma once

#include "Collection.h"
#include "Texture.h"
#include "ShaderProgram.h"
#include "Material.h"
#include "Mesh.h"

class ResourceManager
{
public:
	Collection<Texture, 32> textures;
	Collection<ShaderProgram, 32> shaders;
	Collection<Material, 32> materials;
	Collection<Mesh, 32> meshes;
};
