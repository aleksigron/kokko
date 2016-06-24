#include "ResourceManager.hpp"

#include <cstring>

#include "Shader.hpp"
#include "Material.hpp"
#include "Texture.hpp"
#include "ImageData.hpp"

#include "MemoryAmount.hpp"
#include "File.hpp"
#include "Hash.hpp"

ResourceManager::ResourceManager() : stackAllocator(32_MB, 256_kB)
{

}

ResourceManager::~ResourceManager()
{
	
}
Shader* ResourceManager::GetShader(uint32_t hash) const
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

Shader* ResourceManager::GetShader(const char* path)
{
	size_t shaderNameLen = std::strlen(path);
	uint32_t shaderNameHash = Hash::FNV1a_32(path, shaderNameLen);

	// Try to find the shader using shader path hash
	Shader* result = this->GetShader(shaderNameHash);

	if (result == nullptr)
	{
		if (shaderCount == shaderAllocated)
		{
			unsigned int newAllocatedCount = (shaderAllocated > 0) ? shaderAllocated * 2 : 32;
			Shader* newShaders = new Shader[newAllocatedCount];

			for (unsigned int i = 0; i < shaderCount; ++i)
			{
				newShaders[i] = shaders[i];
			}

			delete[] shaders;
			shaders = newShaders;
			shaderAllocated = newAllocatedCount;
		}

		Shader& shader = shaders[shaderCount];
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

bool ResourceManager::LoadShader(Shader& shader, const char* configPath)
{
	Buffer<char> configuration = File::ReadText(configPath);

	return shader.LoadFromConfiguration(configuration);
}

Material* ResourceManager::GetMaterial(uint32_t hash) const
{
	// Try to find the material using material path hash
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

Texture* ResourceManager::GetTexture(uint32_t hash) const
{
	// Try to find the texture using texture path hash
	for (unsigned int i = 0; i < textureCount; ++i)
	{
		if (textures[i].nameHash == hash)
		{
			return textures + i;
		}
	}

	return nullptr;
}

Texture* ResourceManager::GetTexture(const char* path)
{
	size_t textureNameLen = std::strlen(path);
	uint32_t textureNameHash = Hash::FNV1a_32(path, textureNameLen);

	// Try to find the shader using shader path hash
	Texture* result = this->GetTexture(textureNameHash);

	if (result == nullptr) // Shader not yet loaded
	{
		Texture* texture = this->CreateTexture();

		if (this->LoadTexture(texture, path))
		{
			texture->nameHash = textureNameHash;
			result = texture;
		}
		else
		{
			texture->~Texture();
			--textureCount;
		}
	}
	
	return result;
}

Texture* ResourceManager::CreateTexture()
{
	Texture* result = nullptr;

	if (textureCount == textureAllocated)
	{
		unsigned int newAllocatedCount = (textureAllocated > 0) ? textureAllocated * 2 : 32;
		Texture* newTextures = new Texture[newAllocatedCount];

		for (unsigned int i = 0; i < textureCount; ++i)
		{
			newTextures[i] = textures[i];
		}

		delete[] textures; // Deleting a null pointer is a no-op
		textures = newTextures;
		textureAllocated = newAllocatedCount;
	}

	result = textures + textureCount;
	++textureCount;

	return result;
}

bool ResourceManager::LoadTexture(Texture* texture, const char* path)
{
	ImageData imageData;

	if (imageData.LoadGlraw(path))
	{
		texture->Upload(imageData);

		return true;
	}
	else
		return false;
}
