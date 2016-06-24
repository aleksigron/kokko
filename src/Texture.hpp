#pragma once

#include <cstdint>

#include "Vec2.hpp"

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
	ClampToBorder
};

struct TextureOptions
{
	TextureFilterMode filter = TextureFilterMode::Linear;
	TextureWrapMode wrap = TextureWrapMode::Repeat;
};

struct Texture
{
	uint32_t nameHash;
	unsigned int targetType;
	unsigned int driverId;
	Vec2f textureSize;

	void Upload(const ImageData& image);
	void Upload(const ImageData& image, const TextureOptions& options);
};
