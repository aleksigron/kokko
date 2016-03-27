#pragma once

#include "StackAllocator.hpp"
#include "Collection.hpp"
#include "Texture.hpp"
#include "Mesh.hpp"

struct ShaderProgram;
struct Material;

class ResourceManager
{
private:
	StackAllocator stackAllocator;

	ShaderProgram* shaders = nullptr;
	unsigned int shaderCount = 0;
	unsigned int shaderAllocated = 0;

	bool LoadShader(ShaderProgram& shader, const char* configPath);

	Material* materials = nullptr;
	unsigned int materialCount = 0;
	unsigned int materialAllocated = 0;

	bool LoadMaterial(Material& material, const char* configPath);

public:
	ResourceManager();
	~ResourceManager();

	Collection<Texture, 32> textures;
	Collection<Mesh, 32> meshes;

	ShaderProgram* GetShader(uint32_t hash) const;
	ShaderProgram* GetShader(const char* path);

	Material* GetMaterial(uint32_t hash) const;
	Material* GetMaterial(const char* path);
};
