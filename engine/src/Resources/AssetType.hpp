#pragma once

#include <cstdint>

namespace kokko
{

enum class AssetType : uint8_t
{
	Unknown,
	Level,
	Material,
	Model,
	Shader,
	Texture
};

struct TextureAssetMetadata
{
	bool generateMipmaps = true;
	bool preferLinear = false;
};

}
