#pragma once

#include "StackAllocator.hpp"
#include "IndexedContainer.hpp"

#include "Mesh.hpp"
#include "Material.hpp"

struct Shader;
struct Texture;

class ResourceManager
{
private:
	StackAllocator stackAllocator;

	// Meshes

	IndexedContainer<Mesh> meshes;

	bool LoadMesh(Mesh& mesh, const char* path);

	// Shaders

	Shader* shaders = nullptr;
	unsigned int shaderCount = 0;
	unsigned int shaderAllocated = 0;

	bool LoadShader(Shader& shader, const char* configPath);

	// Materials

	IndexedContainer<Material> materials;

	bool LoadMaterial(Material& material, const char* configPath);

	// Textures

	Texture* textures = nullptr;
	unsigned int textureCount = 0;
	unsigned int textureAllocated = 0;

	bool LoadTexture(Texture* texture, const char* texturePath);

public:
	ResourceManager();
	~ResourceManager();

	Mesh& GetMesh(unsigned int id);
	unsigned int CreateMesh();
	unsigned int CreateMeshFromFile(const char* path);

	Shader* GetShader(uint32_t hash) const;
	Shader* GetShader(const char* path);

	Material& GetMaterial(unsigned int id);
	unsigned int CreateMaterialFromFile(const char* path);

	Texture* GetTexture(uint32_t hash) const;
	Texture* GetTexture(const char* path);
	Texture* CreateTexture();
};
