#pragma once

#include <cstdint>

#include "Core/ArrayView.hpp"
#include "Core/HashMap.hpp"
#include "Core/StringView.hpp"
#include "Core/Uid.hpp"

#include "Math/Vec2.hpp"

#include "Rendering/RenderTypes.hpp"
#include "Rendering/RenderResourceId.hpp"

#include "Resources/TextureId.hpp"

class Allocator;
struct ImageData;

namespace kokko
{
class AssetLoader;

namespace render
{
class Device;
}

struct TextureData
{
	kokko::Uid uid;
	Vec2i textureSize;
	kokko::render::TextureId textureObjectId;
	RenderTextureTarget textureTarget;
};

class TextureManager
{
private:
	Allocator* allocator;
	kokko::AssetLoader* assetLoader;
	kokko::render::Device* renderDevice;

	struct InstanceData
	{
		unsigned int count;
		unsigned int allocated;
		void* buffer;

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
	TextureManager(Allocator* allocator, kokko::AssetLoader* assetLoader, kokko::render::Device* renderDevice);
	~TextureManager();

	void Initialize();

	TextureId CreateTexture();
	void RemoveTexture(TextureId id);

	// TODO: Create a way for asset loader or some other mechanism to provide the metadata
	TextureId FindTextureByUid(const kokko::Uid& uid, bool preferLinear = false);
	TextureId FindTextureByPath(kokko::ConstStringView path, bool preferLinear = false);

	TextureId GetId_White2D() const { return constantTextures[ConstTex_White2D]; }
	TextureId GetId_Black2D() const { return constantTextures[ConstTex_Black2D]; }
	TextureId GetId_EmptyNormal() const { return constantTextures[ConstTex_EmptyNormal]; }

	const TextureData& GetTextureData(TextureId id) { return data.texture[id.i]; }

	void Upload_2D(TextureId id, const ImageData& image, bool generateMipmaps);
	void Upload_Cube(TextureId id, const ImageData* images, bool generateMipmaps);

	void AllocateTextureStorage(TextureId id,
		RenderTextureTarget target, RenderTextureSizedFormat format, int levels, Vec2i size);

private:
	bool LoadWithStbImage(TextureId id, ArrayView<const uint8_t> bytes, bool preferLinear);
};

} // namespace kokko
