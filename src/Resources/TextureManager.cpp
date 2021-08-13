#include "Resources/TextureManager.hpp"

#include <cassert>

#include "rapidjson/document.h"
#include "stb_image/stb_image.h"

#include "Core/Core.hpp"
#include "Core/Hash.hpp"
#include "Core/String.hpp"

#include "Rendering/RenderDevice.hpp"

#include "Resources/ImageData.hpp"

#include "System/File.hpp"
#include "System/IncludeOpenGL.hpp"

TextureId TextureId::Null = TextureId{ 0 };

TextureManager::TextureManager(Allocator* allocator, RenderDevice* renderDevice) :
	allocator(allocator),
	renderDevice(renderDevice),
	nameHashMap(allocator)
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
	for (unsigned int i = 0; i < data.allocated; ++i)
		if (data.texture[i].textureObjectId != 0)
			renderDevice->DestroyTextures(1, &(data.texture[i].textureObjectId));

	allocator->Deallocate(data.buffer);
}

void TextureManager::Initialize()
{
	KOKKO_PROFILE_FUNCTION();

	static const unsigned int size = 16;
	static const unsigned int bytesPerPixel = 3;
	unsigned char buffer[size * size * bytesPerPixel];

	ImageData imageData;
	imageData.imageData = buffer;
	imageData.imageDataSize = sizeof(buffer);

	imageData.imageSize = Vec2i(size, size);
	imageData.pixelFormat = GL_RGB;
	imageData.componentDataType = GL_UNSIGNED_BYTE;

	TextureOptions options;
	options.minFilter = RenderTextureFilterMode::Nearest;
	options.magFilter = RenderTextureFilterMode::Nearest;

	{
		std::memset(buffer, 255, sizeof(buffer));
		TextureId white2d = CreateTexture();
		Upload_2D(white2d, imageData, options);
		constantTextures[ConstTex_White2D] = white2d;
	}

	{
		std::memset(buffer, 0, sizeof(buffer));
		TextureId black2d = CreateTexture();
		Upload_2D(black2d, imageData, options);
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
		Upload_2D(emptyNormal, imageData, options);
		constantTextures[ConstTex_EmptyNormal] = emptyNormal;
	}
}

void TextureManager::Reallocate(unsigned int required)
{
	KOKKO_PROFILE_FUNCTION();

	if (required <= data.allocated)
		return;

	required = Math::UpperPowerOfTwo(required);

	size_t objectBytes = sizeof(unsigned int) + sizeof(TextureData);

	InstanceData newData;
	newData.buffer = allocator->Allocate(required * objectBytes);
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
	// Mesh isn't the last one
	if (id.i < data.count - 1)
	{
		data.freeList[id.i] = freeListFirst;
		freeListFirst = id.i;
	}

	if (data.texture[id.i].textureObjectId != 0)
	{
		unsigned int objectId = data.texture[id.i].textureObjectId;
		renderDevice->DestroyTextures(1, &objectId);
		data.texture[id.i].textureObjectId = 0;
	}

	--data.count;
}

TextureId TextureManager::GetIdByPath(StringRef path)
{
	uint32_t hash = Hash::FNV1a_32(path.str, path.len);

	HashMap<uint32_t, TextureId>::KeyValuePair* pair = nameHashMap.Lookup(hash);
	if (pair != nullptr)
		return pair->second;

	if (data.count == data.allocated)
		this->Reallocate(data.count + 1);

	String pathStr(allocator, path);

	TextureId id = CreateTexture();

	if (path.EndsWith(StringRef(".jpg")) ||
		path.EndsWith(StringRef(".jpeg")) ||
		path.EndsWith(StringRef(".png")))
	{
		if (LoadWithStbImage(id, pathStr.GetCStr()))
		{
			pair = nameHashMap.Insert(hash);
			pair->second = id;

			return id;
		}
	}

	RemoveTexture(id);
	return TextureId::Null;
}

bool TextureManager::LoadWithStbImage(TextureId id, const char* filePath, bool preferLinear)
{
	stbi_set_flip_vertically_on_load(true);
	int width, height, nrComponents;
	uint8_t* textureBytes;
	{
		KOKKO_PROFILE_SCOPE("void* stbi_loadf()");
		textureBytes = stbi_load(filePath, &width, &height, &nrComponents, 0);
	}

	unsigned int textureObjectId = 0;

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
			sizedFormat = preferLinear ? RenderTextureSizedFormat::RGB8 : RenderTextureSizedFormat::SRGB8;
			baseFormat = RenderTextureBaseFormat::RGB;
			break;

		case 4:
			formatFound = true;
			sizedFormat = preferLinear ? RenderTextureSizedFormat::RGBA8 : RenderTextureSizedFormat::SRGB8_A8;
			baseFormat = RenderTextureBaseFormat::RGBA;
			break;

		default:
			KK_LOG_ERROR("Invalid number of components in texture: %d", nrComponents);
			break;
		}

		if (formatFound)
		{
			KOKKO_PROFILE_SCOPE("Create and upload texture");

			renderDevice->CreateTextures(1, &textureObjectId);
			renderDevice->BindTexture(RenderTextureTarget::Texture2d, textureObjectId);

			RenderCommandData::SetTextureStorage2D textureStorage{
				RenderTextureTarget::Texture2d, 1, sizedFormat, width, height,
			};
			renderDevice->SetTextureStorage2D(&textureStorage);

			RenderCommandData::SetTextureSubImage2D textureImage{
				RenderTextureTarget::Texture2d, 0, 0, 0, width, height,
				baseFormat, RenderTextureDataType::UnsignedByte, textureBytes
			};
			renderDevice->SetTextureSubImage2D(&textureImage);
		}

		{
			KOKKO_PROFILE_SCOPE("void stbi_image_free()");
			stbi_image_free(textureBytes);
		}

		TextureData& textureData = data.texture[id.i];
		textureData.textureSize = Vec2i(width, height);
		textureData.textureObjectId = textureObjectId;
		textureData.textureTarget = RenderTextureTarget::Texture2d;
	}
	else
	{
		KK_LOG_ERROR("Couldn't load texture file with stb_image");

		return -1;
	}
}

void TextureManager::Upload_2D(TextureId id, const ImageData& image, const TextureOptions& options)
{
	KOKKO_PROFILE_FUNCTION();

	TextureData& texture = data.texture[id.i];

	texture.textureSize = image.imageSize;

	texture.textureTarget = RenderTextureTarget::Texture2d;

	renderDevice->CreateTextures(1, &texture.textureObjectId);
	renderDevice->BindTexture(texture.textureTarget, texture.textureObjectId);

	if (image.compressed)
	{
		RenderCommandData::SetTextureImageCompressed2D textureImage{
			texture.textureTarget, 0, image.pixelFormat, image.imageSize.x,
			image.imageSize.y, image.compressedSize, image.imageData
		};

		renderDevice->SetTextureImageCompressed2D(&textureImage);
	}
	else
	{
		RenderCommandData::SetTextureImage2D textureImage{
			texture.textureTarget, 0, image.pixelFormat, image.imageSize.x, image.imageSize.y,
			image.pixelFormat, image.componentDataType, image.imageData
		};

		renderDevice->SetTextureImage2D(&textureImage);
	}

	if (options.generateMipmaps)
	{
		renderDevice->GenerateTextureMipmaps(texture.textureTarget);
	}

	renderDevice->SetTextureMinFilter(texture.textureTarget, options.minFilter);
	renderDevice->SetTextureMagFilter(texture.textureTarget, options.magFilter);

	renderDevice->SetTextureWrapModeU(texture.textureTarget, options.wrapModeU);
	renderDevice->SetTextureWrapModeV(texture.textureTarget, options.wrapModeV);
}

void TextureManager::Upload_Cube(TextureId id, const ImageData* images, const TextureOptions& options)
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

	renderDevice->CreateTextures(1, &texture.textureObjectId);
	renderDevice->BindTexture(texture.textureTarget, texture.textureObjectId);

	for (unsigned int i = 0; i < 6; ++i)
	{
		const ImageData& image = images[i];
		RenderTextureTarget targetFace = cubemapFaceValues[i];

		if (image.compressed)
		{
			RenderCommandData::SetTextureImageCompressed2D textureImage{
				targetFace, 0, image.pixelFormat, image.imageSize.x,
				image.imageSize.y, image.compressedSize, image.imageData
			};

			renderDevice->SetTextureImageCompressed2D(&textureImage);
		}
		else
		{
			RenderCommandData::SetTextureImage2D textureImage{
				targetFace, 0, image.pixelFormat, image.imageSize.x, image.imageSize.y,
				image.pixelFormat, image.componentDataType, image.imageData
			};

			renderDevice->SetTextureImage2D(&textureImage);
		}

		if (options.generateMipmaps)
		{
			renderDevice->GenerateTextureMipmaps(texture.textureTarget);
		}

		renderDevice->SetTextureMinFilter(texture.textureTarget, options.minFilter);
		renderDevice->SetTextureMagFilter(texture.textureTarget, options.magFilter);

		renderDevice->SetTextureWrapModeU(texture.textureTarget, options.wrapModeU);
		renderDevice->SetTextureWrapModeV(texture.textureTarget, options.wrapModeV);
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

		renderDevice->CreateTextures(1, &texture.textureObjectId);
		renderDevice->BindTexture(target, texture.textureObjectId);

		RenderCommandData::SetTextureStorage2D textureStorage{ target, levels, format, size.x, size.y };
		renderDevice->SetTextureStorage2D(&textureStorage);

		if (target == RenderTextureTarget::TextureCubeMap)
		{
			renderDevice->SetTextureWrapModeU(target, RenderTextureWrapMode::ClampToEdge);
			renderDevice->SetTextureWrapModeV(target, RenderTextureWrapMode::ClampToEdge);
			renderDevice->SetTextureWrapModeW(target, RenderTextureWrapMode::ClampToEdge);
		}
	}
	else
	{
		KK_LOG_ERROR("Can't allocate texture storage again");
	}
}
