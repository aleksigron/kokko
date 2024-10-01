#include "Resources/TextureManager.hpp"

#include <cassert>

#include "rapidjson/document.h"
#include "stb_image/stb_image.h"

#include "Core/Core.hpp"
#include "Core/Hash.hpp"
#include "Core/String.hpp"

#include "Rendering/RenderDevice.hpp"

#include "Resources/AssetLoader.hpp"
#include "Resources/ImageData.hpp"

#include "System/IncludeOpenGL.hpp"

namespace kokko
{

namespace
{
int MipLevelsFromDimensions(int width, int height)
{
	uint32_t smaller = static_cast<uint32_t>(width > height ? height : width);

	int levels = 1;
	while ((smaller >>= 1) > 0)
		levels += 1;

	return levels;
}

Optional<RenderTextureSizedFormat> SizedFormatFromBaseFormatAndType(
	RenderTextureBaseFormat base,
	RenderTextureDataType type,
	bool preferLinear)
{
	bool formatFound = false;

	if (base == RenderTextureBaseFormat::R)
	{
		if (type == RenderTextureDataType::UnsignedByte || type == RenderTextureDataType::SignedByte)
			return RenderTextureSizedFormat::R8;
		else if (type == RenderTextureDataType::UnsignedShort || type == RenderTextureDataType::SignedShort)
			return RenderTextureSizedFormat::R16;
		else if (type == RenderTextureDataType::UnsignedInt || type == RenderTextureDataType::SignedInt ||
			type == RenderTextureDataType::Float)
			return RenderTextureSizedFormat::R32F;
	}
	else if (base == RenderTextureBaseFormat::RG)
	{
		if (type == RenderTextureDataType::UnsignedByte || type == RenderTextureDataType::SignedByte)
			return RenderTextureSizedFormat::RG8;
		else if (type == RenderTextureDataType::UnsignedShort || type == RenderTextureDataType::SignedShort)
			return RenderTextureSizedFormat::RG16;
		else if (type == RenderTextureDataType::UnsignedInt || type == RenderTextureDataType::SignedInt ||
			type == RenderTextureDataType::Float)
			return RenderTextureSizedFormat::RG32F;
	}
	else if (base == RenderTextureBaseFormat::RGB)
	{
		if (type == RenderTextureDataType::UnsignedByte || type == RenderTextureDataType::SignedByte)
			return preferLinear ? RenderTextureSizedFormat::RGB8 : RenderTextureSizedFormat::SRGB8;
		else if (type == RenderTextureDataType::UnsignedShort || type == RenderTextureDataType::SignedShort)
			return RenderTextureSizedFormat::RGB16;
		else if (type == RenderTextureDataType::UnsignedInt || type == RenderTextureDataType::SignedInt ||
			type == RenderTextureDataType::Float)
			return RenderTextureSizedFormat::RGB32F;
	}
	else if (base == RenderTextureBaseFormat::RGBA)
	{
		if (type == RenderTextureDataType::UnsignedByte || type == RenderTextureDataType::SignedByte)
			return preferLinear ? RenderTextureSizedFormat::RGBA8 : RenderTextureSizedFormat::SRGB8_A8;
		else if (type == RenderTextureDataType::UnsignedShort || type == RenderTextureDataType::SignedShort)
			return RenderTextureSizedFormat::RGBA16;
		else if (type == RenderTextureDataType::UnsignedInt || type == RenderTextureDataType::SignedInt ||
			type == RenderTextureDataType::Float)
			return RenderTextureSizedFormat::RGBA32F;
	}

	KK_LOG_ERROR("Couldn't find sized texture format");
	return Optional<RenderTextureSizedFormat>();
}

} // namespace

TextureId TextureId::Null = TextureId{ 0 };

TextureManager::TextureManager(Allocator* allocator, kokko::AssetLoader* assetLoader, kokko::render::Device* renderDevice) :
	allocator(allocator),
	assetLoader(assetLoader),
	renderDevice(renderDevice),
	uidMap(allocator)
{
	data = InstanceData{};
	data.count = 1; // Reserve index 0 as Null instance

	freeListFirst = 0;

	for (unsigned int i = 0; i < ConstTex_Count; ++i)
		constantTextures[i] = TextureId{ 0 };

	this->Reallocate(32);
}

TextureManager::~TextureManager()
{
	for (unsigned int i = 1; i < data.allocated; ++i)
		if (data.texture[i].textureObjectId != 0)
			renderDevice->DestroyTextures(1, &(data.texture[i].textureObjectId));

	allocator->Deallocate(data.buffer);
}

void TextureManager::Initialize()
{
	KOKKO_PROFILE_FUNCTION();

	auto scope = renderDevice->CreateDebugScope(0, kokko::ConstStringView("TextureMan_InitResources"));

	static const unsigned int size = 16;
	static const unsigned int bytesPerPixel = 3;
	unsigned char buffer[size * size * bytesPerPixel];

	ImageData imageData;
	imageData.imageData = buffer;
	imageData.imageDataSize = sizeof(buffer);

	imageData.imageSize = Vec2i(size, size);
	imageData.pixelFormat = RenderTextureBaseFormat::RGB;
	imageData.componentDataType = RenderTextureDataType::UnsignedByte;

	{
		std::memset(buffer, 255, sizeof(buffer));
		TextureId white2d = CreateTexture();
		Upload_2D(white2d, imageData, false);
		constantTextures[ConstTex_White2D] = white2d;
	}

	{
		std::memset(buffer, 0, sizeof(buffer));
		TextureId black2d = CreateTexture();
		Upload_2D(black2d, imageData, false);
		constantTextures[ConstTex_Black2D] = black2d;
	}

	{
		for (unsigned int i = 0, count = size * size; i < count; ++i)
		{
			buffer[i * bytesPerPixel + 0] = 128;
			buffer[i * bytesPerPixel + 1] = 128;
			buffer[i * bytesPerPixel + 2] = 255;
		}

		TextureId emptyNormal = CreateTexture();
		Upload_2D(emptyNormal, imageData, false);
		constantTextures[ConstTex_EmptyNormal] = emptyNormal;
	}
}

void TextureManager::Reallocate(unsigned int required)
{
	KOKKO_PROFILE_FUNCTION();

	if (required <= data.allocated)
		return;

	required = static_cast<unsigned int>(Math::UpperPowerOfTwo(required));

	size_t objectBytes = sizeof(unsigned int) + sizeof(TextureData);

	InstanceData newData;
	newData.buffer = allocator->Allocate(required * objectBytes, "TextureManager.data.buffer");
	newData.count = data.count;
	newData.allocated = required;

	newData.freeList = static_cast<unsigned int*>(newData.buffer);
	newData.texture = reinterpret_cast<TextureData*>(newData.freeList + required);

	if (data.buffer != nullptr)
	{
		std::memcpy(newData.freeList, data.freeList, data.allocated * sizeof(unsigned int));
		std::memcpy(newData.texture, data.texture, data.count * sizeof(TextureData));

		allocator->Deallocate(data.buffer);
	}

	for (unsigned int i = data.allocated, end = newData.allocated; i < end; ++i)
	{
		newData.texture[i].textureObjectId = kokko::render::TextureId();
	}

	data = newData;
}


TextureId TextureManager::CreateTexture()
{
	TextureId id;

	if (freeListFirst == 0)
	{
		if (data.count == data.allocated)
			this->Reallocate(data.count + 1);

		// If there are no freelist entries, first <objectCount> indices must be in use
		id.i = data.count;
	}
	else
	{
		id.i = freeListFirst;
		freeListFirst = data.freeList[freeListFirst];
	}

	// Clear buffer data
	data.texture[id.i] = TextureData{};

	++data.count;

	return id;
}

void TextureManager::RemoveTexture(TextureId id)
{
	assert(id != TextureId::Null);

	auto mapPair = uidMap.Lookup(data.texture[id.i].uid);
	if (mapPair != nullptr)
		uidMap.Remove(mapPair);

	// Mesh isn't the last one
	if (id.i < data.count - 1)
	{
		data.freeList[id.i] = freeListFirst;
		freeListFirst = id.i;
	}

	if (data.texture[id.i].textureObjectId != 0)
	{
		render::TextureId objectId = data.texture[id.i].textureObjectId;
		renderDevice->DestroyTextures(1, &objectId);
		data.texture[id.i].textureObjectId = render::TextureId();
	}

	--data.count;
}

TextureId TextureManager::FindTextureByUid(const kokko::Uid& uid)
{
	KOKKO_PROFILE_FUNCTION();

	if (uid.raw[0] == 0 && uid.raw[1] == 0)
		KK_LOG_WARN("TextureManager::FindTextureByUid called with zero UID");

	auto* pair = uidMap.Lookup(uid);
	if (pair != nullptr)
		return pair->second;

	if (data.count == data.allocated)
		this->Reallocate(data.count + 1);

	Array<uint8_t> buffer(allocator);
	AssetLoader::LoadResult loadResult = assetLoader->LoadAsset(uid, buffer);
	if (loadResult.success)
	{
		assert(loadResult.assetType == AssetType::Texture);

		TextureAssetMetadata metadata;
		if (loadResult.metadataSize == sizeof(metadata))
			memcpy(&metadata, buffer.GetData(), loadResult.metadataSize);

		TextureId id = CreateTexture();

		auto assetView = buffer.GetSubView(loadResult.assetStart, loadResult.assetStart + loadResult.assetSize);
		if (LoadWithStbImage(id, assetView, metadata))
		{
			data.texture[id.i].uid = uid;

			pair = uidMap.Insert(uid);
			pair->second = id;

			return id;
		}
		else
		{
			KK_LOG_ERROR("Texture failed to load correctly");

			RemoveTexture(id);
		}
	}
	else
		KK_LOG_ERROR("AssetLoader couldn't load texture asset");

	return TextureId::Null;
}

TextureId TextureManager::FindTextureByPath(kokko::ConstStringView path)
{
	KOKKO_PROFILE_FUNCTION();

	auto uidResult = assetLoader->GetAssetUidByVirtualPath(path);
	if (uidResult.HasValue())
	{
		return FindTextureByUid(uidResult.GetValue());
	}

	return TextureId::Null;
}

bool TextureManager::LoadWithStbImage(TextureId id, ArrayView<const uint8_t> bytes, TextureAssetMetadata metadata)
{
	stbi_set_flip_vertically_on_load(true);
	int width, height, nrComponents;
	uint8_t* textureBytes;

	{
		KOKKO_PROFILE_SCOPE("stbi_load_from_memory()");

		const uint8_t* fileBytesPtr = bytes.GetData();
		int length = static_cast<int>(bytes.GetCount());
		textureBytes = stbi_load_from_memory(fileBytesPtr, length, &width, &height, &nrComponents, 0);
	}

	kokko::render::TextureId textureObjectId;

	if (textureBytes != nullptr)
	{
		bool formatFound = false;
		RenderTextureSizedFormat sizedFormat;
		RenderTextureBaseFormat baseFormat;

		switch (nrComponents)
		{
		case 1:
			formatFound = true;
			sizedFormat = RenderTextureSizedFormat::R8;
			baseFormat = RenderTextureBaseFormat::R;
			break;

		case 2:
			formatFound = true;
			sizedFormat = RenderTextureSizedFormat::RG8;
			baseFormat = RenderTextureBaseFormat::RG;
			break;

		case 3:
			formatFound = true;
			sizedFormat = metadata.preferLinear ? RenderTextureSizedFormat::RGB8 : RenderTextureSizedFormat::SRGB8;
			baseFormat = RenderTextureBaseFormat::RGB;
			break;

		case 4:
			formatFound = true;
			sizedFormat = metadata.preferLinear ? RenderTextureSizedFormat::RGBA8 : RenderTextureSizedFormat::SRGB8_A8;
			baseFormat = RenderTextureBaseFormat::RGBA;
			break;

		default:
			KK_LOG_ERROR("Invalid number of components in texture: %d", nrComponents);
			break;
		}

		if (formatFound)
		{
			KOKKO_PROFILE_SCOPE("Create and upload texture");

			int mipLevels = metadata.generateMipmaps ? MipLevelsFromDimensions(width, height) : 1;

			renderDevice->CreateTextures(RenderTextureTarget::Texture2d, 1, &textureObjectId);
			renderDevice->SetTextureStorage2D(textureObjectId, mipLevels, sizedFormat, width, height);
			renderDevice->SetTextureSubImage2D(textureObjectId, 0, 0, 0, width, height,
				baseFormat, RenderTextureDataType::UnsignedByte, textureBytes);

			if (mipLevels > 1)
			{
				KOKKO_PROFILE_SCOPE("Generate texture mipmaps");
				renderDevice->GenerateTextureMipmaps(textureObjectId);
			}
		}

		{
			KOKKO_PROFILE_SCOPE("void stbi_image_free()");
			stbi_image_free(textureBytes);
		}

		if (formatFound)
		{
			TextureData& textureData = data.texture[id.i];
			textureData.textureSize = Vec2i(width, height);
			textureData.textureObjectId = textureObjectId;
			textureData.textureTarget = RenderTextureTarget::Texture2d;

			return true;
		}
	}
	else
		KK_LOG_ERROR("Couldn't load texture file with stb_image");

	return false;
}

void TextureManager::Upload_2D(TextureId id, const ImageData& image, bool generateMipmaps)
{
	KOKKO_PROFILE_FUNCTION();

	TextureData& texture = data.texture[id.i];

	assert(texture.textureObjectId == 0);

	texture.textureSize = image.imageSize;
	texture.textureTarget = RenderTextureTarget::Texture2d;

	const int mips = generateMipmaps ? MipLevelsFromDimensions(image.imageSize.x, image.imageSize.y) : 1;

	renderDevice->CreateTextures(texture.textureTarget, 1, &texture.textureObjectId);

	const Optional<RenderTextureSizedFormat> format =
		SizedFormatFromBaseFormatAndType(image.pixelFormat, image.componentDataType, true);
	assert(format.HasValue());

	renderDevice->SetTextureStorage2D(
		texture.textureObjectId, mips, format.GetValue(), image.imageSize.x, image.imageSize.y);

	if (image.compressed)
	{
		assert(false && "Compressed textures not yet supported");

		/*
		RenderCommandData::SetTextureImageCompressed2D textureImage{
			texture.textureTarget, 0, image.pixelFormat,
			image.imageSize.x, image.imageSize.y,
			static_cast<unsigned int>(image.compressedSize), image.imageData
		};

		renderDevice->SetTextureSubImageCompressed2D(texture.textureObjectId, 0, image.pixelFormat,
			image.imageSize.x, image.imageSize.y,
			static_cast<unsigned int>(image.compressedSize), image.imageData);
		*/
	}
	else
	{
		renderDevice->SetTextureSubImage2D(texture.textureObjectId, 0, 0, 0, image.imageSize.x, image.imageSize.y,
			image.pixelFormat, image.componentDataType, image.imageData);
	}

	if (generateMipmaps)
		renderDevice->GenerateTextureMipmaps(texture.textureObjectId);
}

void TextureManager::Upload_Cube(TextureId id, const ImageData* images, bool generateMipmaps)
{
	KOKKO_PROFILE_FUNCTION();

	static const RenderTextureTarget cubemapFaceValues[] = {
		RenderTextureTarget::TextureCubeMap_PositiveX,
		RenderTextureTarget::TextureCubeMap_NegativeX,
		RenderTextureTarget::TextureCubeMap_PositiveY,
		RenderTextureTarget::TextureCubeMap_NegativeY,
		RenderTextureTarget::TextureCubeMap_PositiveZ,
		RenderTextureTarget::TextureCubeMap_NegativeZ
	};

	TextureData& texture = data.texture[id.i];

	// There's really no reason to use different size faces
	// We can simply use the size of the first image
	texture.textureSize = images[0].imageSize;

	texture.textureTarget = RenderTextureTarget::TextureCubeMap;

	renderDevice->CreateTextures(RenderTextureTarget::TextureCubeMap, 1, &texture.textureObjectId);

	const int mips = generateMipmaps ? MipLevelsFromDimensions(images[0].imageSize.x, images[0].imageSize.y) : 1;
	const Optional<RenderTextureSizedFormat> format =
		SizedFormatFromBaseFormatAndType(images[0].pixelFormat, images[0].componentDataType, true);
	assert(format.HasValue());

	renderDevice->SetTextureStorage2D(
		texture.textureObjectId, mips, format.GetValue(), texture.textureSize.x, texture.textureSize.y);

	for (int face = 0; face < 6; ++face)
	{
		const ImageData& image = images[face];

		if (image.compressed)
		{
			assert(false && "Compressed textures not yet supported");

			/*
			RenderCommandData::SetTextureImageCompressed2D textureImage{
				targetFace, 0, image.pixelFormat, image.imageSize.x, image.imageSize.y,
				static_cast<unsigned int>(image.compressedSize), image.imageData
			};

			renderDevice->SetTextureImageCompressed2D(&textureImage);
			*/
		}
		else
		{
			renderDevice->SetTextureSubImage3D(
				texture.textureObjectId, 0, 0, 0, face, image.imageSize.x, image.imageSize.y,
				1, image.pixelFormat, image.componentDataType, image.imageData);
		}

		if (generateMipmaps)
			renderDevice->GenerateTextureMipmaps(texture.textureObjectId);
	}
}

void TextureManager::AllocateTextureStorage(TextureId id, RenderTextureTarget target, RenderTextureSizedFormat format, int levels, Vec2i size)
{
	KOKKO_PROFILE_FUNCTION();

	TextureData& texture = data.texture[id.i];

	if (texture.textureObjectId == 0)
	{
		texture.textureSize = size;
		texture.textureTarget = target;

		renderDevice->CreateTextures(target, 1, &texture.textureObjectId);
		renderDevice->SetTextureStorage2D(texture.textureObjectId, levels, format, size.x, size.y);
		/*
		if (target == RenderTextureTarget::TextureCubeMap)
		{
			renderDevice->SetTextureWrapModeU(target, RenderTextureWrapMode::ClampToEdge);
			renderDevice->SetTextureWrapModeV(target, RenderTextureWrapMode::ClampToEdge);
			renderDevice->SetTextureWrapModeW(target, RenderTextureWrapMode::ClampToEdge);
		}
		*/
	}
	else
	{
		KK_LOG_ERROR("Can't allocate texture storage again");
	}
}

void TextureManager::Update()
{
	Array<uint8_t> buffer(allocator);

	Uid uid;
	while (assetLoader->GetNextUpdatedAssetUid(AssetType::Texture, uid))
	{
		auto* pair = uidMap.Lookup(uid);
		if (pair != nullptr)
		{
			TextureId id{ pair->second };

			buffer.Clear();
			AssetLoader::LoadResult loadResult = assetLoader->LoadAsset(uid, buffer);
			if (loadResult.success)
			{
				if (data.texture[id.i].textureObjectId != render::TextureId::Null)
				{
					renderDevice->DestroyTextures(1, &data.texture[id.i].textureObjectId);
					data.texture[id.i].textureObjectId = render::TextureId::Null;
				}

				assert(loadResult.assetType == AssetType::Texture);

				TextureAssetMetadata metadata;
				if (loadResult.metadataSize == sizeof(metadata))
					memcpy(&metadata, buffer.GetData(), loadResult.metadataSize);

				bool preferLinear = false;
				auto assetView = buffer.GetSubView(loadResult.assetStart, loadResult.assetStart + loadResult.assetSize);
				if (LoadWithStbImage(id, assetView, metadata) == false)
				{
					KK_LOG_ERROR("Texture failed to update");
				}
			}
		}
		else
		{
			FindTextureByUid(uid);
		}

	}
}

} // namespace kokko
