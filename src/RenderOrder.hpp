#pragma once

#include <cstdint>

#include "BitfieldVariable.hpp"

namespace RenderOrder
{
	enum ViewportIndex
	{
		Viewport_Fullscreen = 0
	};

	enum Command
	{
		Command_Control = 0,
		Command_Draw = 1
	};

	enum ControlCommandType
	{
		Control_GlEnable,
		Control_GlDisable,
		Control_BlendingEnable,
		Control_BlendingDisable,
		Control_DepthTestEnable,
		Control_DepthTestDisable,
		Control_DepthWriteEnable,
		Control_DepthWriteDisable
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
