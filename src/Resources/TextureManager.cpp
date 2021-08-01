#include "Resources/TextureManager.hpp"

#include <cassert>

#include "rapidjson/document.h"
#include "ktx.h"

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

	if (LoadFromKtxFile(id, pathStr.GetCStr()))
	{
		pair = nameHashMap.Insert(hash);
		pair->second = id;

		return id;
	}
	else
	{
		RemoveTexture(id);
		return TextureId{};
	}
}

bool TextureManager::LoadFromKtxFile(TextureId id, const char* ktxFilePath)
{
	KOKKO_PROFILE_FUNCTION();

	TextureData& textureData = data.texture[id.i];

	ktxTexture* kTexture;
	KTX_error_code result;

	result = ktxTexture_CreateFromNamedFile(ktxFilePath, KTX_TEXTURE_CREATE_NO_FLAGS, &kTexture);

	assert(result == KTX_SUCCESS);

	GLuint textureName = 0;
	GLenum target;

	if (kTexture->classId == ktxTexture2_c)
	{
		ktxTexture2* k2Texture = reinterpret_cast<ktxTexture2*>(kTexture);

		if (ktxTexture2_NeedsTranscoding(k2Texture))
		{
			ktx_texture_transcode_fmt_e tf = KTX_TTF_BC1_OR_3;

			result = ktxTexture2_TranscodeBasis(k2Texture, tf, 0);
			assert(result == KTX_SUCCESS);
		}
	}


	result = ktxTexture_GLUpload(kTexture, &textureName, &target, nullptr);

	assert(result == KTX_SUCCESS);

	ktxTexture_Destroy(kTexture);

	textureData.textureObjectId = textureName;

	switch (target)
	{
	case GL_TEXTURE_1D:
		textureData.textureTarget = RenderTextureTarget::Texture1d;
		break;

	case GL_TEXTURE_2D:
		textureData.textureTarget = RenderTextureTarget::Texture2d;
		break;

	case GL_TEXTURE_3D:
		textureData.textureTarget = RenderTextureTarget::Texture3d;
		break;

	case GL_TEXTURE_1D_ARRAY:
		textureData.textureTarget = RenderTextureTarget::Texture1dArray;
		break;

	case GL_TEXTURE_2D_ARRAY:
		textureData.textureTarget = RenderTextureTarget::Texture2dArray;
		break;

	case GL_TEXTURE_CUBE_MAP:
		textureData.textureTarget = RenderTextureTarget::TextureCubeMap;
		break;
	}

	return true;
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
