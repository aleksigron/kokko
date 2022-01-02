#pragma once

#include <cstdint>

#include "Core/ArrayView.hpp"
#include "Core/HashMap.hpp"
#include "Core/StringRef.hpp"
#include "Core/Uid.hpp"

#include "Math/Vec2.hpp"

#include "Rendering/RenderDeviceEnums.hpp"

#include "Resources/TextureId.hpp"

class Allocator;
class RenderDevice;
struct ImageData;

namespace kokko
{
class AssetLoader;
}

struct TextureData
{
	kokko::Uid uid;
	Vec2i textureSize;
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
	kokko::AssetLoader* assetLoader;
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
	HashMap<kokko::Uid, TextureId> uidMap;

	enum ConstantTextures
	{
		ConstTex_White2D,
		ConstTex_Black2D,
		ConstTex_EmptyNormal,

		ConstTex_Count
	};

	TextureId constantTextures[ConstTex_Count];

	void Reallocate(unsigned int required);

public:
	TextureManager(Allocator* allocator, kokko::AssetLoader* assetLoader, RenderDevice* renderDevice);
	~TextureManager();

	void Initialize();

	TextureId CreateTexture();
	void RemoveTexture(TextureId id);
	
	TextureId FindTextureByUid(const kokko::Uid& uid);
	TextureId FindTextureByPath(const StringRef& path);

	TextureId GetId_White2D() const { return constantTextures[ConstTex_White2D]; }
	TextureId GetId_Black2D() const { return constantTextures[ConstTex_Black2D]; }
	TextureId GetId_EmptyNormal() const { return constantTextures[ConstTex_EmptyNormal]; }

	const TextureData& GetTextureData(TextureId id) { return data.texture[id.i]; }

	void Upload_2D(TextureId id, const ImageData& image, const TextureOptions& options);
	void Upload_Cube(TextureId id, const ImageData* images, const TextureOptions& options);

	void AllocateTextureStorage(TextureId id,
		RenderTextureTarget target, RenderTextureSizedFormat format, int levels, Vec2i size);

private:
	bool LoadWithStbImage(TextureId id, ArrayView<const uint8_t> bytes, bool preferLinear = false);
};
