#include "ResourceManager.hpp"

#include <cstring>

#include "Rendering/Shader.hpp"
#include "Texture.hpp"
#include "ImageData.hpp"

#include "System/File.hpp"
#include "Core/Hash.hpp"

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
	delete[] shaders;
	delete[] textures;
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

	if (configuration.IsValid())
		return shader.LoadFromConfiguration(configuration.GetRef());
	else
		return false;
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

	// Try to find the texture using hash
	Texture* result = this->GetTexture(textureNameHash);

	if (result == nullptr) // Texture not yet loaded
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
	Buffer<char> textureConfig = File::ReadText(path);

	if (textureConfig.IsValid())
	{
		return texture->LoadFromConfiguration(textureConfig.GetRef());
	}

	return false;
}
