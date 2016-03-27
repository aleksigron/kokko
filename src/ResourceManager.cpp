#include "ResourceManager.hpp"

#include <cstring>

#include "ShaderProgram.hpp"
#include "Material.hpp"

#include "MemoryAmount.hpp"
#include "File.hpp"
#include "Hash.hpp"

ResourceManager::ResourceManager() : stackAllocator(32_MB, 256_kB)
{

}

ResourceManager::~ResourceManager()
{
	
}
ShaderProgram* ResourceManager::GetShader(uint32_t hash) const
{
	for (unsigned int i = 0; i < shaderCount; ++i)
	{
		if (shaders[i].nameHash == hash)
		{
			return shaders + i;
		}
	}

	return nullptr;
}

ShaderProgram* ResourceManager::GetShader(const char* path)
{
	size_t shaderNameLen = std::strlen(path);
	uint32_t shaderNameHash = Hash::FNV1a_32(path, shaderNameLen);

	// Try to find the shader using shader path hash
	ShaderProgram* result = this->GetShader(shaderNameHash);

	if (result == nullptr)
	{
		if (shaderCount == shaderAllocated)
		{
			unsigned int newAllocatedCount = (shaderAllocated > 0) ? shaderAllocated * 2 : 32;
			ShaderProgram* newShaders = new ShaderProgram[newAllocatedCount];

			for (unsigned int i = 0; i < shaderCount; ++i)
			{
				newShaders[i] = shaders[i];
			}

			delete[] shaders;
			shaders = newShaders;
			shaderAllocated = newAllocatedCount;
		}

		ShaderProgram& shader = shaders[shaderCount];
		shader.SetAllocator(&stackAllocator);

		if (this->LoadShader(shader, path))
		{
			++shaderCount;

			shader.nameHash = shaderNameHash;

			result = &shader;
		}
	}

	return result;
}

bool ResourceManager::LoadShader(ShaderProgram& shader, const char* configPath)
{
	Buffer<char> configuration = File::ReadText(configPath);

	return shader.LoadFromConfiguration(configuration);
}

Material* ResourceManager::GetMaterial(uint32_t hash) const
{
	// Try to find the shader using shader path hash
	for (unsigned int i = 0; i < materialCount; ++i)
	{
		if (materials[i].nameHash == hash)
		{
			return materials + i;
		}
	}

	return nullptr;
}

Material* ResourceManager::GetMaterial(const char* path)
{
	size_t materialNameLen = std::strlen(path);
	uint32_t materialNameHash = Hash::FNV1a_32(path, materialNameLen);

	// Try to find the shader using shader path hash
	Material* result = this->GetMaterial(materialNameHash);

	if (result == nullptr) // Shader not yet loaded
	{
		if (materialCount == materialAllocated)
		{
			unsigned int newAllocatedCount = (materialAllocated > 0) ? materialAllocated * 2 : 32;
			Material* newMaterials = new Material[newAllocatedCount];

			for (unsigned int i = 0; i < materialCount; ++i)
			{
				newMaterials[i] = materials[i];
			}

			delete[] materials; // Deleting a null pointer is a no-op
			materials = newMaterials;
			materialAllocated = newAllocatedCount;
		}

		Material& material = materials[materialCount];

		if (this->LoadMaterial(material, path))
		{
			++materialCount;

			material.nameHash = materialNameHash;

			result = &material;
		}
	}

	return result;
}

bool ResourceManager::LoadMaterial(Material& material, const char* configPath)
{
	Buffer<char> configuration = File::ReadText(configPath);

	return material.LoadFromConfiguration(configuration, this);
}
