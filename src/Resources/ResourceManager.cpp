#include "Resources/ResourceManager.hpp"

#include <cstring>

#include "Core/Hash.hpp"

#include "Resources/ShaderLoader.hpp"
#include "Resources/Texture.hpp"
#include "Resources/ImageData.hpp"

#include "System/File.hpp"

ResourceManager::ResourceManager(Allocator* allocator, RenderDevice* renderDevice) :
	allocator(allocator),
	renderDevice(renderDevice),
	textures(nullptr),
	textureCount(0),
	textureAllocated(0)
{
}

ResourceManager::~ResourceManager()
{
	allocator->Deallocate(textures);
}

RenderDevice* ResourceManager::GetRenderDevice()
{
	return renderDevice;
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
