#pragma once

#include "StackAllocator.hpp"
#include "Collection.hpp"
#include "Texture.hpp"
#include "Mesh.hpp"

struct Shader;
struct Material;

class ResourceManager
{
private:
	StackAllocator stackAllocator;

	Shader* shaders = nullptr;
	unsigned int shaderCount = 0;
	unsigned int shaderAllocated = 0;

	bool LoadShader(Shader& shader, const char* configPath);

	Material* materials = nullptr;
	unsigned int materialCount = 0;
	unsigned int materialAllocated = 0;

	bool LoadMaterial(Material& material, const char* configPath);

public:
	ResourceManager();
	~ResourceManager();

	Collection<Texture, 32> textures;
	Collection<Mesh, 32> meshes;

	Shader* GetShader(uint32_t hash) const;
	Shader* GetShader(const char* path);

	Material* GetMaterial(uint32_t hash) const;
	Material* GetMaterial(const char* path);
};
