#pragma once

#include "Collection.hpp"
#include "Texture.hpp"
#include "ShaderProgram.hpp"
#include "Material.hpp"
#include "Mesh.hpp"

class ResourceManager
{
public:
	Collection<Texture, 32> textures;
	Collection<ShaderProgram, 32> shaders;
	Collection<Material, 32> materials;
	Collection<Mesh, 32> meshes;
};
