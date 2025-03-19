#pragma once

namespace kokko
{

enum class TransparencyType : unsigned char
{
	Opaque = 0,
	AlphaTest = 1,
	Skybox = 2,
	TransparentMix = 3,
	TransparentAdd = 4,
	TransparentSub = 5
};

} // namespace kokko
