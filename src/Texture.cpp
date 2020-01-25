#include "Texture.hpp"

#include "IncludeOpenGL.hpp"

#include "rapidjson/document.h"

#include "File.hpp"
#include "Core/Hash.hpp"
#include "Core/StringRef.hpp"
#include "ImageData.hpp"

static int GetFilterModeValue(TextureFilterMode mode)
{
	static const int values[] = {
		GL_NEAREST,
		GL_LINEAR
	};

	return values[static_cast<unsigned long>(mode)];
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

static unsigned int GetTargetType(TextureType type)
{
	static const unsigned int values[] = {
		0,
		GL_TEXTURE_2D,
		GL_TEXTURE_CUBE_MAP
	};

	return values[static_cast<unsigned long>(type)];
}

bool Texture::LoadFromConfiguration(BufferRef<char> configuration)
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
		case "tex2d"_hash: textureType = TextureType::Texture2D; break;
		case "texCube"_hash: textureType = TextureType::TextureCube; break;
		default: textureType = TextureType::Undefined; break;
	}

	targetType = GetTargetType(textureType);

	MemberIterator textureItr = config.FindMember("texture");
	assert(textureItr != config.MemberEnd());

	const rapidjson::Value& textureValue = textureItr->value;

	if (textureType == TextureType::Texture2D)
	{
		assert(textureValue.IsString());

		const char* texturePath = textureValue.GetString();
		Buffer<unsigned char> textureContent = File::ReadBinary(texturePath);

		if (textureContent.IsValid())
		{
			ImageData image;
			if (image.LoadGlraw(textureContent.GetRef()))
			{
				this->Upload_2D(image);

				return true;
			}
		}
	}
	else if (textureType == TextureType::TextureCube)
	{
		assert(textureValue.IsArray());
		assert(textureValue.Size() == 6);

		bool allTextureLoadsSucceeded = true;
		Buffer<unsigned char> fileContents[6];
		ImageData cubeFaceImages[6];

		for (unsigned int i = 0, count = textureValue.Size(); i < count; ++i)
		{
			assert(textureValue[i].IsString());

			const char* texturePath = textureValue[i].GetString();
			fileContents[i] = File::ReadBinary(texturePath);

			if (fileContents[i].IsValid())
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
			this->Upload_Cube(cubeFaceImages);

			return true;
		}
	}

	return false;
}

void Texture::Upload_2D(const ImageData& image)
{
	this->Upload_2D(image, TextureOptions());
}

void Texture::Upload_2D(const ImageData& image, const TextureOptions& options)
{
	textureSize.x = static_cast<float>(image.imageSize.x);
	textureSize.y = static_cast<float>(image.imageSize.y);

	targetType = GL_TEXTURE_2D;

	glGenTextures(1, &driverId);
	glBindTexture(targetType, driverId);

	if (image.compressed)
	{
		// Upload compressed texture data
		glCompressedTexImage2D(targetType, 0, image.pixelFormat,
							   image.imageSize.x, image.imageSize.y, 0,
							   image.compressedSize, image.imageData);
	}
	else
	{
		// Upload uncompressed texture data
		glTexImage2D(targetType, 0, image.pixelFormat,
					 image.imageSize.x, image.imageSize.y, 0,
					 image.pixelFormat, image.componentDataType, image.imageData);
	}

	int filterMode = GetFilterModeValue(options.filter);

	// Set filter
	glTexParameteri(targetType, GL_TEXTURE_MIN_FILTER, filterMode);
	glTexParameteri(targetType, GL_TEXTURE_MAG_FILTER, filterMode);

	int wrapMode = GetWrapModeValue(options.wrap);

	glTexParameteri(targetType, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(targetType, GL_TEXTURE_WRAP_T, wrapMode);

	// Unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Upload_Cube(const ImageData* images)
{
	TextureOptions options;
	options.wrap = TextureWrapMode::ClampToEdge;

	this->Upload_Cube(images, options);
}

void Texture::Upload_Cube(const ImageData* images, const TextureOptions& options)
{
	static const unsigned int cubemapFaceValues[] = {
		GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	};

	int filterMode = GetFilterModeValue(options.filter);
	int wrapMode = GetWrapModeValue(options.wrap);

	// There's really no reason to use different size faces
	// We can simply use the size of the first image
	textureSize.x = static_cast<float>(images[0].imageSize.x);
	textureSize.y = static_cast<float>(images[0].imageSize.y);

	targetType = GL_TEXTURE_CUBE_MAP;

	glGenTextures(1, &driverId);
	glBindTexture(targetType, driverId);

	for (unsigned int i = 0; i < 6; ++i)
	{
		const ImageData& image = images[i];
		unsigned int targetFace = cubemapFaceValues[i];

		if (image.compressed)
		{
			// Upload compressed texture data
			glCompressedTexImage2D(targetFace, 0, image.pixelFormat,
								   image.imageSize.x, image.imageSize.y, 0,
								   image.compressedSize, image.imageData);
		}
		else
		{
			// Upload uncompressed texture data
			glTexImage2D(targetFace, 0, image.pixelFormat,
						 image.imageSize.x, image.imageSize.y, 0,
						 image.pixelFormat, image.componentDataType, image.imageData);
		}

		// Set filter mode
		glTexParameteri(targetType, GL_TEXTURE_MIN_FILTER, filterMode);
		glTexParameteri(targetType, GL_TEXTURE_MAG_FILTER, filterMode);

		// Set wrap mode
		glTexParameteri(targetType, GL_TEXTURE_WRAP_S, wrapMode);
		glTexParameteri(targetType, GL_TEXTURE_WRAP_T, wrapMode);
	}
}








