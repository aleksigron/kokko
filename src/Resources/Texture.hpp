#pragma once

#include <cstdint>

#include "Math/Vec2.hpp"
#include "Core/BufferRef.hpp"

class Allocator;

struct ImageData;

enum class TextureFilterMode
{
	Nearest,
	Linear
};

enum class TextureWrapMode
{
	Repeat,
	MirroredRepeat,
	ClampToEdge,
};

enum class TextureType
{
	Undefined,
	Texture2D,
	TextureCube
};

struct TextureOptions
{
	TextureFilterMode filter = TextureFilterMode::Linear;
	TextureWrapMode wrap = TextureWrapMode::Repeat;
};

struct Texture
{
	uint32_t nameHash;

	TextureType textureType;
	unsigned int targetType;

	unsigned int driverId;
	Vec2f textureSize;

	bool LoadFromConfiguration(BufferRef<char> configuration, Allocator* allocator);

	void Upload_2D(const ImageData& image);
	void Upload_2D(const ImageData& image, const TextureOptions& options);

	void Upload_Cube(const ImageData* images);
	void Upload_Cube(const ImageData* images, const TextureOptions& options);
};