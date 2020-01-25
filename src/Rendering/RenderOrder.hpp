#pragma once

#include <cstdint>

#include "Core/BitfieldVariable.hpp"

struct RenderOrderConfiguration
{
	RenderOrderConfiguration()
	{
		viewportIndex.SetDefinition(3, sizeof(uint64_t) * 8);
		viewportPass.SetDefinition(3, viewportIndex.shift);
		command.SetDefinition(1, viewportPass.shift);

		// DRAW COMMANDS

		// For transparents

		transparentDepth.SetDefinition(22, command.shift);

		// For opaques

		opaqueDepth.SetDefinition(8, command.shift);
		opaquePadding.SetDefinition(14, opaqueDepth.shift);

		// For all draw commands

		materialId.SetDefinition(12, opaquePadding.shift);
		renderObject.SetDefinition(20, materialId.shift);

		// CONTROL COMMANDS

		// Ordering for commands for same viewport, layer and transparency
		commandOrder.SetDefinition(4, command.shift);

		// Type of command
		commandType.SetDefinition(8, commandOrder.shift);

		// Command data or offset to buffer, depending on command type
		commandData.SetDefinition(32, commandType.shift);

		// Calculate maximum integer depth values in advance

		maxTransparentDepth = (1 << transparentDepth.bits) - 1;
		maxOpaqueDepth = (1 << opaqueDepth.bits) - 1;
	}

	BitfieldVariable<uint64_t> viewportIndex;
	BitfieldVariable<uint64_t> viewportPass;
	BitfieldVariable<uint64_t> command;
	BitfieldVariable<uint64_t> transparentDepth;
	BitfieldVariable<uint64_t> opaqueDepth;
	BitfieldVariable<uint64_t> opaquePadding;
	BitfieldVariable<uint64_t> materialId;
	BitfieldVariable<uint64_t> renderObject;
	BitfieldVariable<uint64_t> commandOrder;
	BitfieldVariable<uint64_t> commandType;
	BitfieldVariable<uint64_t> commandData;

	uint64_t maxTransparentDepth;
	uint64_t maxOpaqueDepth;
};
