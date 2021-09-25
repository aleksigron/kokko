#pragma once

enum class TransparencyType : unsigned char
{
	Opaque = 0,
	AlphaTest = 1,
	// Deferred lighting pass for opaque geometry
	Skybox = 3,
	TransparentMix = 4,
	TransparentAdd = 5,
	TransparentSub = 6
};
