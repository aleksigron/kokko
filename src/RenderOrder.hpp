#pragma once

#include <cstdint>

#include "BitfieldVariable.hpp"

struct DrawCall
{
	uint64_t orderKey;
	unsigned int renderObjectIndex;
};

namespace RenderOrder
{
	enum ViewportIndex
	{
		FullscreenViewport = 0
	};

	enum ViewportLayer
	{
		SkyboxLayer = 0,
		WorldLayer = 1
	};

	enum TransparencyType
	{
		Opaque = 0,
		AlphaTest = 1,
		Transparent = 2
	};

	enum Command
	{
		ControlCommand = 0,
		DrawCommand = 1
	};

	enum ControlCommandType
	{
		GlEnable,
		GlDisable,
		DepthWriteEnable,
		DepthWriteDisable
	};
}

struct RenderOrderConfiguration
{
	BitfieldVariable<uint64_t> viewportIndex;
	BitfieldVariable<uint64_t> viewportLayer;
	BitfieldVariable<uint64_t> transparencyType;
	BitfieldVariable<uint64_t> command;
	BitfieldVariable<uint64_t> transparentDepth;
	BitfieldVariable<uint64_t> transparentMaterialId;
	BitfieldVariable<uint64_t> opaqueMaterialId;
	BitfieldVariable<uint64_t> opaqueDepth;
	BitfieldVariable<uint64_t> commandType;
	BitfieldVariable<uint64_t> commandData;
};
