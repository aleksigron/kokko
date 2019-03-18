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
	RenderOrderConfiguration()
	{
		viewportIndex.SetDefinition(3, sizeof(uint64_t) * 8);
		viewportLayer.SetDefinition(3, viewportIndex.shift);
		transparencyType.SetDefinition(3, viewportLayer.shift);
		command.SetDefinition(1, transparencyType.shift);

		// For non-command transparents

		transparentDepth.SetDefinition(22, command.shift);

		// For non-command opaques

		opaqueDepth.SetDefinition(8, command.shift);
		opaquePadding.SetDefinition(14, opaqueDepth.shift);

		// For all non-commands

		materialId.SetDefinition(12, opaquePadding.shift);
		renderObject.SetDefinition(20, materialId.shift);

		// For commands

		commandType.SetDefinition(8, command.shift);
		commandData.SetDefinition(32, commandType.shift);

		maxTransparentDepth = (1 << transparentDepth.bits) - 1;
		maxOpaqueDepth = (1 << opaqueDepth.bits) - 1;
	}

	BitfieldVariable<uint64_t> viewportIndex;
	BitfieldVariable<uint64_t> viewportLayer;
	BitfieldVariable<uint64_t> transparencyType;
	BitfieldVariable<uint64_t> command;
	BitfieldVariable<uint64_t> transparentDepth;
	BitfieldVariable<uint64_t> opaqueDepth;
	BitfieldVariable<uint64_t> opaquePadding;
	BitfieldVariable<uint64_t> materialId;
	BitfieldVariable<uint64_t> renderObject;
	BitfieldVariable<uint64_t> commandType;
	BitfieldVariable<uint64_t> commandData;

	uint64_t maxTransparentDepth;
	uint64_t maxOpaqueDepth;
};
