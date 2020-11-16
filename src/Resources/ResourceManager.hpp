#pragma once

#include <cstdint>

class Allocator;
class RenderDevice;
struct Shader;
struct Texture;

class ResourceManager
{
private:
	Allocator* allocator;
	RenderDevice* renderDevice;

	Texture* textures;
	unsigned int textureCount;
	unsigned int textureAllocated;

	bool LoadTexture(Texture* texture, const char* texturePath);

public:
	ResourceManager(Allocator* allocator, RenderDevice* renderDevice);
	~ResourceManager();

	RenderDevice* GetRenderDevice();

	Texture* GetTexture(uint32_t hash) const;
	Texture* GetTexture(const char* path);
	Texture* CreateTexture();
};
