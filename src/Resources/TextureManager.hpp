#pragma once

#include <cstdint>

#include "Core/HashMap.hpp"
#include "Core/BufferRef.hpp"
#include "Core/StringRef.hpp"

#include "Math/Vec2.hpp"

#include "Rendering/RenderDeviceEnums.hpp"

class Allocator;
class RenderDevice;
struct ImageData;

struct TextureId
{
	unsigned int i;

	bool IsNull() const { return i == 0; }
};

struct TextureData
{
	Vec2f textureSize;
	unsigned int textureObjectId;
	RenderTextureTarget textureTarget;
};

struct TextureOptions
{
	RenderTextureFilterMode minFilter = RenderTextureFilterMode::Linear;
	RenderTextureFilterMode magFilter = RenderTextureFilterMode::Linear;
	RenderTextureWrapMode wrapModeU = RenderTextureWrapMode::Repeat;
	RenderTextureWrapMode wrapModeV = RenderTextureWrapMode::Repeat;
	bool generateMipmaps = false;
};

class TextureManager
{
private:
	Allocator* allocator;

	RenderDevice* renderDevice;

	struct InstanceData
	{
		unsigned int count;
		unsigned int allocated;
		void *buffer;

		unsigned int* freeList;
		TextureData* texture;
	}
	data;

	unsigned int freeListFirst;
	HashMap<uint32_t, TextureId> nameHashMap;

	void Reallocate(unsigned int required);

public:
	TextureManager(Allocator* allocator, RenderDevice* renderDevice);
	~TextureManager();

	TextureId CreateTexture();
	void RemoveTexture(TextureId id);
	
	TextureId GetIdByPath(StringRef path);
	TextureId GetIdByPathHash(uint32_t pathHash)
	{
		auto pair = nameHashMap.Lookup(pathHash);
		return pair != nullptr ? pair->second : TextureId{};
	}

	const TextureData& GetTextureData(TextureId id) { return data.texture[id.i]; }

	bool LoadFromKtxFile(TextureId id, const char* ktxFilePath);

	void Upload_2D(TextureId id, const ImageData& image, const TextureOptions& options);
	void Upload_Cube(TextureId id, const ImageData* images, const TextureOptions& options);
};
