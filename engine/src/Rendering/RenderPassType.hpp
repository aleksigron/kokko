#pragma once

namespace kokko
{

enum class RenderPassType
{
	Setup = 0,
	OpaqueGeometry = 1, // Opaque + AlphaTest
	OpaqueLighting = 2,
	Skybox = 3,
	Transparent = 4, // TransparentMix + TransparentAdd + TransparentSub
	PostProcess = 5
};

}
