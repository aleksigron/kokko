#include "Resources/Texture.hpp"

#include "rapidjson/document.h"

#include "Core/Hash.hpp"
#include "Core/StringRef.hpp"

#include "Rendering/RenderDevice.hpp"

#include "Resources/ImageData.hpp"

#include "System/File.hpp"
#include "System/IncludeOpenGL.hpp"

static TextureFilterMode ParseFilterModeString(const char* str, std::size_t strLen)
{
	uint32_t hash = Hash::FNV1a_32(str, strLen);

	switch (hash)
	{
	case "nearest"_hash: return TextureFilterMode::Nearest;
	case "linear"_hash: return TextureFilterMode::Linear;
	case "linear_mipmap"_hash: return TextureFilterMode::LinearMipmap;
	default: return TextureFilterMode::Linear;
	}
}

static int GetFilterModeValue(TextureFilterMode mode)
{
	static const int values[] = {
		GL_NEAREST,
		GL_LINEAR,
		GL_LINEAR_MIPMAP_LINEAR
	};

	return values[static_cast<unsigned long>(mode)];
}

static TextureWrapMode ParseWrapModeString(const char* str, std::size_t strLen)
{
	uint32_t hash = Hash::FNV1a_32(str, strLen);

	switch (hash)
	{
	case "repeat"_hash: return TextureWrapMode::Repeat;
	case "mirror"_hash: return TextureWrapMode::MirroredRepeat;
	case "clamp"_hash: return TextureWrapMode::ClampToEdge;
	default: return TextureWrapMode::Repeat;
	}
}

static int GetWrapModeValue(TextureWrapMode mode)
{
	static const int values[] = {
		GL_REPEAT,
		GL_MIRRORED_REPEAT,
		GL_CLAMP_TO_EDGE
	};

	return values[static_cast<unsigned long>(mode)];
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

bool Texture::LoadFromConfiguration(
	BufferRef<char> configuration,
	Allocator* allocator,
	RenderDevice* renderDevice)
{
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
		case "tex1d"_hash: textureTarget = RenderTextureTarget::Texture1d; break;
		case "tex2d"_hash: textureTarget = RenderTextureTarget::Texture2d; break;
		case "texCube"_hash: textureTarget = RenderTextureTarget::TextureCubeMap; break;
		default: 
			return false;
	}

	MemberIterator textureItr = config.FindMember("texture");
	assert(textureItr != config.MemberEnd());

	const rapidjson::Value& textureValue = textureItr->value;

	if (textureTarget == RenderTextureTarget::Texture2d)
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

				this->Upload_2D(renderDevice, image, opts);

				return true;
			}
		}
	}
	else if (textureTarget == RenderTextureTarget::TextureCubeMap)
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

			this->Upload_Cube(renderDevice, cubeFaceImages, opts);

			return true;
		}
	}

	return false;
}

void Texture::Upload_2D(RenderDevice* renderDevice, const ImageData& image, const TextureOptions& options)
{
	textureSize.x = static_cast<float>(image.imageSize.x);
	textureSize.y = static_cast<float>(image.imageSize.y);

	textureTarget = RenderTextureTarget::Texture2d;

	renderDevice->CreateTextures(1, &driverId);
	renderDevice->BindTexture(textureTarget, driverId);

	if (image.compressed)
	{
		RenderCommandData::SetTextureImageCompressed2D textureImage{
			textureTarget, 0, image.pixelFormat, image.imageSize.x,
			image.imageSize.y, image.compressedSize, image.imageData
		};

		renderDevice->SetTextureImageCompressed2D(&textureImage);
	}
	else
	{
		RenderCommandData::SetTextureImage2D textureImage{
			textureTarget, 0, image.pixelFormat, image.imageSize.x, image.imageSize.y,
			image.pixelFormat, image.componentDataType, image.imageData
		};

		renderDevice->SetTextureImage2D(&textureImage);
	}

	if (options.generateMipmaps)
	{
		renderDevice->GenerateTextureMipmaps(textureTarget);
	}

	int minFilter = GetFilterModeValue(options.minFilter);
	int magFilter = GetFilterModeValue(options.magFilter);

	renderDevice->SetTextureParameterInt(textureTarget, GL_TEXTURE_MIN_FILTER, minFilter);
	renderDevice->SetTextureParameterInt(textureTarget, GL_TEXTURE_MAG_FILTER, magFilter);

	int wrapModeU = GetWrapModeValue(options.wrapModeU);
	int wrapModeV = GetWrapModeValue(options.wrapModeV);

	renderDevice->SetTextureParameterInt(textureTarget, GL_TEXTURE_WRAP_S, wrapModeU);
	renderDevice->SetTextureParameterInt(textureTarget, GL_TEXTURE_WRAP_T, wrapModeV);
}

void Texture::Upload_Cube(RenderDevice* renderDevice, const ImageData* images, const TextureOptions& options)
{
	static const RenderTextureTarget cubemapFaceValues[] = {
		RenderTextureTarget::TextureCubeMap_PositiveX,
		RenderTextureTarget::TextureCubeMap_NegativeX,
		RenderTextureTarget::TextureCubeMap_PositiveY,
		RenderTextureTarget::TextureCubeMap_NegativeY,
		RenderTextureTarget::TextureCubeMap_PositiveZ,
		RenderTextureTarget::TextureCubeMap_NegativeZ
	};

	int minFilter = GetFilterModeValue(options.minFilter);
	int magFilter = GetFilterModeValue(options.magFilter);
	int wrapModeU = GetWrapModeValue(options.wrapModeU);
	int wrapModeV = GetWrapModeValue(options.wrapModeV);

	// There's really no reason to use different size faces
	// We can simply use the size of the first image
	textureSize.x = static_cast<float>(images[0].imageSize.x);
	textureSize.y = static_cast<float>(images[0].imageSize.y);

	textureTarget = RenderTextureTarget::TextureCubeMap;

	renderDevice->CreateTextures(1, &driverId);
	renderDevice->BindTexture(textureTarget, driverId);

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
			renderDevice->GenerateTextureMipmaps(textureTarget);
		}

		renderDevice->SetTextureParameterInt(textureTarget, GL_TEXTURE_MIN_FILTER, minFilter);
		renderDevice->SetTextureParameterInt(textureTarget, GL_TEXTURE_MAG_FILTER, magFilter);

		renderDevice->SetTextureParameterInt(textureTarget, GL_TEXTURE_WRAP_S, wrapModeU);
		renderDevice->SetTextureParameterInt(textureTarget, GL_TEXTURE_WRAP_T, wrapModeV);
	}
}








