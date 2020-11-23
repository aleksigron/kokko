#include "Resources/TextureManager.hpp"

#include <cassert>

#include "rapidjson/document.h"

#include "Core/Hash.hpp"
#include "Core/String.hpp"

#include "Rendering/RenderDevice.hpp"

#include "Resources/ImageData.hpp"

#include "System/File.hpp"

static RenderTextureFilterMode ParseFilterModeString(const char* str, std::size_t strLen)
{
	uint32_t hash = Hash::FNV1a_32(str, strLen);

	switch (hash)
	{
	case "nearest"_hash: return RenderTextureFilterMode::Nearest;
	case "linear"_hash: return RenderTextureFilterMode::Linear;
	case "linear_mipmap"_hash: return RenderTextureFilterMode::LinearMipmap;
	default: return RenderTextureFilterMode::Linear;
	}
}

static RenderTextureWrapMode ParseWrapModeString(const char* str, std::size_t strLen)
{
	uint32_t hash = Hash::FNV1a_32(str, strLen);

	switch (hash)
	{
	case "repeat"_hash: return RenderTextureWrapMode::Repeat;
	case "mirror"_hash: return RenderTextureWrapMode::MirroredRepeat;
	case "clamp"_hash: return RenderTextureWrapMode::ClampToEdge;
	default: return RenderTextureWrapMode::Repeat;
	}
}

static TextureOptions GetOptionsFromJson(const rapidjson::Value& json)
{
	using MemberIterator = rapidjson::Value::ConstMemberIterator;

	TextureOptions opts;

	MemberIterator minfItr = json.FindMember("min_filter");
	if (minfItr != json.MemberEnd() && minfItr->value.IsString())
	{
		opts.minFilter = ParseFilterModeString(
			minfItr->value.GetString(), minfItr->value.GetStringLength());
	}

	MemberIterator magfItr = json.FindMember("mag_filter");
	if (magfItr != json.MemberEnd() && magfItr->value.IsString())
	{
		opts.magFilter = ParseFilterModeString(
			magfItr->value.GetString(), magfItr->value.GetStringLength());
	}

	MemberIterator wmuItr = json.FindMember("wrap_mode_u");
	if (wmuItr != json.MemberEnd() && wmuItr->value.IsString())
	{
		opts.wrapModeU = ParseWrapModeString(
			wmuItr->value.GetString(), wmuItr->value.GetStringLength());
	}

	MemberIterator wmvItr = json.FindMember("wrap_mode_v");
	if (wmvItr != json.MemberEnd() && wmvItr->value.IsString())
	{
		opts.wrapModeV = ParseWrapModeString(
			wmvItr->value.GetString(), wmvItr->value.GetStringLength());
	}

	MemberIterator gmmItr = json.FindMember("generate_mipmaps");
	if (gmmItr != json.MemberEnd() && gmmItr->value.IsBool())
	{
		opts.generateMipmaps = gmmItr->value.GetBool();
	}

	return opts;
}

TextureManager::TextureManager(Allocator* allocator, RenderDevice* renderDevice) :
	allocator(allocator),
	renderDevice(renderDevice),
	nameHashMap(allocator)
{
	data = InstanceData{};
	data.count = 1; // Reserve index 0 as Null instance

	freeListFirst = 0;

	this->Reallocate(32);
}

TextureManager::~TextureManager()
{
	for (unsigned int i = 0; i < data.allocated; ++i)
		if (data.texture[i].textureObjectId != 0)
			renderDevice->DestroyTextures(1, &(data.texture[i].textureObjectId));

	allocator->Deallocate(data.buffer);
}

void TextureManager::Reallocate(unsigned int required)
{
	if (required <= data.allocated)
		return;

	required = Math::UpperPowerOfTwo(required);

	unsigned int objectBytes = sizeof(unsigned int) + sizeof(TextureData);

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

	Buffer<char> file(allocator);
	String pathStr(allocator, path);

	if (File::ReadText(pathStr.GetCStr(), file))
	{
		TextureId id = CreateTexture();

		if (LoadFromConfiguration(id, file.GetRef()))
		{
			pair = nameHashMap.Insert(hash);
			pair->second = id;

			return id;
		}
		else
		{
			RemoveTexture(id);
		}
	}

	return TextureId{};
}

bool TextureManager::LoadFromConfiguration(TextureId id, BufferRef<char> configuration)
{
	TextureData& texture = data.texture[id.i];

	rapidjson::Document config;
	config.ParseInsitu(configuration.data);

	assert(config.HasMember("type"));

	using MemberIterator = rapidjson::Value::ConstMemberIterator;
	MemberIterator typeItr = config.FindMember("type");

	assert(typeItr != config.MemberEnd());
	assert(typeItr->value.IsString());

	StringRef typeName;
	typeName.str = typeItr->value.GetString();
	typeName.len = typeItr->value.GetStringLength();

	uint32_t typeHash = Hash::FNV1a_32(typeName.str, typeName.len);

	switch (typeHash)
	{
	case "tex1d"_hash: texture.textureTarget = RenderTextureTarget::Texture1d; break;
	case "tex2d"_hash: texture.textureTarget = RenderTextureTarget::Texture2d; break;
	case "texCube"_hash: texture.textureTarget = RenderTextureTarget::TextureCubeMap; break;
	default:
		return false;
	}

	MemberIterator textureItr = config.FindMember("texture");
	assert(textureItr != config.MemberEnd());

	const rapidjson::Value& textureValue = textureItr->value;

	if (texture.textureTarget == RenderTextureTarget::Texture2d)
	{
		assert(textureValue.IsString());

		const char* texturePath = textureValue.GetString();
		Buffer<unsigned char> textureContent(allocator);

		if (File::ReadBinary(texturePath, textureContent))
		{
			ImageData image;
			if (image.LoadGlraw(textureContent.GetRef()))
			{
				TextureOptions opts = GetOptionsFromJson(config);

				Upload_2D(id, image, opts);

				return true;
			}
		}
	}
	else if (texture.textureTarget == RenderTextureTarget::TextureCubeMap)
	{
		assert(textureValue.IsArray());
		assert(textureValue.Size() == 6);

		bool allTextureLoadsSucceeded = true;
		Buffer<unsigned char> fileContents[6] =
		{
			Buffer<unsigned char>(allocator),
			Buffer<unsigned char>(allocator),
			Buffer<unsigned char>(allocator),
			Buffer<unsigned char>(allocator),
			Buffer<unsigned char>(allocator),
			Buffer<unsigned char>(allocator)
		};

		ImageData cubeFaceImages[6];

		for (unsigned int i = 0, count = textureValue.Size(); i < count; ++i)
		{
			assert(textureValue[i].IsString());

			const char* texturePath = textureValue[i].GetString();

			if (File::ReadBinary(texturePath, fileContents[i]))
			{
				ImageData& image = cubeFaceImages[i];

				if (image.LoadGlraw(fileContents[i].GetRef()) == false)
				{
					allTextureLoadsSucceeded = false;
				}
			}
			else
			{
				allTextureLoadsSucceeded = false;
			}
		}

		if (allTextureLoadsSucceeded)
		{
			TextureOptions opts = GetOptionsFromJson(config);

			Upload_Cube(id, cubeFaceImages, opts);

			return true;
		}
	}

	return false;
}


void TextureManager::Upload_2D(TextureId id, const ImageData& image, const TextureOptions& options)
{
	TextureData& texture = data.texture[id.i];

	texture.textureSize.x = static_cast<float>(image.imageSize.x);
	texture.textureSize.y = static_cast<float>(image.imageSize.y);

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
	texture.textureSize.x = static_cast<float>(images[0].imageSize.x);
	texture.textureSize.y = static_cast<float>(images[0].imageSize.y);

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