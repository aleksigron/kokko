#pragma once

#include <cstdint>

#include "Core/BufferRef.hpp"

#include "Math/Vec2.hpp"

#include "Rendering/RenderDeviceEnums.hpp"

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

	RenderTextureTarget textureTarget;

	unsigned int driverId;
	Vec2f textureSize;

	bool LoadFromConfiguration(
		BufferRef<char> configuration,
		Allocator* allocator,
		RenderDevice* renderDevice);

	void Upload_2D(RenderDevice* renderDevice, const ImageData& image, const TextureOptions& options);
	void Upload_Cube(RenderDevice* renderDevice, const ImageData* images, const TextureOptions& options);
};
