#pragma once

#include <cstdint>

#include "Math/Vec2.hpp"
#include "Core/BufferRef.hpp"

class Allocator;
class RenderDevice;
struct ImageData;

enum class TextureFilterMode
{
	Nearest,
	Linear,
	LinearMipmap
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
	TextureFilterMode minFilter = TextureFilterMode::Linear;
	TextureFilterMode magFilter = TextureFilterMode::Linear;
	TextureWrapMode wrapModeU = TextureWrapMode::Repeat;
	TextureWrapMode wrapModeV = TextureWrapMode::Repeat;
	bool generateMipmaps = false;
};

struct Texture
{
	uint32_t nameHash;

	TextureType textureType;
	unsigned int targetType;

	unsigned int driverId;
	Vec2f textureSize;

	bool LoadFromConfiguration(
		BufferRef<char> configuration,
		Allocator* allocator,
		RenderDevice* renderDevice);

	void Upload_2D(RenderDevice* renderDevice, const ImageData& image, const TextureOptions& options);
	void Upload_Cube(RenderDevice* renderDevice, const ImageData* images, const TextureOptions& options);
};
