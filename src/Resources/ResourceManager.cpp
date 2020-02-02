#include "Resources/ResourceManager.hpp"

#include <cstring>

#include "Rendering/Shader.hpp"
#include "Resources/Texture.hpp"
#include "Resources/ImageData.hpp"

#include "System/File.hpp"
#include "Core/Hash.hpp"

ResourceManager::ResourceManager(Allocator* allocator, RenderDevice* renderDevice) :
	allocator(allocator),
	renderDevice(renderDevice)
{
}

ResourceManager::~ResourceManager()
{
	allocator->Deallocate(textures);
	allocator->Deallocate(shaders);
}

RenderDevice* ResourceManager::GetRenderDevice()
{
	return renderDevice;
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
			unsigned int newAllocatedSize = newAllocatedCount * sizeof(Shader);
			Shader* newShaders = static_cast<Shader*>(allocator->Allocate(newAllocatedSize));

			for (unsigned int i = 0; i < shaderCount; ++i)
			{
				newShaders[i] = shaders[i];
			}

			allocator->Deallocate(shaders);
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
	Buffer<char> configuration(allocator);

	if (File::ReadText(configPath, configuration))
		return shader.LoadFromConfiguration(configuration.GetRef(), allocator, renderDevice);
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
		unsigned int newAllocatedSize = newAllocatedCount * sizeof(Texture);
		Texture* newTextures = static_cast<Texture*>(allocator->Allocate(newAllocatedSize));

		for (unsigned int i = 0; i < textureCount; ++i)
		{
			newTextures[i] = textures[i];
		}

		allocator->Deallocate(textures); // Deleting a null pointer is a no-op
		textures = newTextures;
		textureAllocated = newAllocatedCount;
	}

	result = textures + textureCount;
	++textureCount;

	return result;
}

bool ResourceManager::LoadTexture(Texture* texture, const char* path)
{
	Buffer<char> textureConfig(allocator);

	if (File::ReadText(path, textureConfig))
	{
		return texture->LoadFromConfiguration(textureConfig.GetRef(), allocator, renderDevice);
	}

	return false;
}
