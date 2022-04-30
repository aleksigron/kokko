#pragma once

enum class RenderPassType
{
	OpaqueGeometry = 0, // Opaque + AlphaTest
	OpaqueLighting = 2,
	Skybox = 3,
	Transparent = 4, // TransparentMix + TransparentAdd + TransparentSub
	PostProcess = 7
};
