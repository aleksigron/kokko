#pragma once

enum class TransparencyType : unsigned char
{
	Opaque = 0,
	AlphaTest = 1,
	TransparentMix = 2,
	TransparentAdd = 3,
	TransparentSub = 4
};
