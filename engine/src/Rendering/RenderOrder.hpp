#pragma once

#include <cstdint>

#include "Core/BitfieldVariable.hpp"

namespace kokko
{

struct RenderOrderConfiguration
{
	static const uint64_t CallbackMaterialId = (1 << 11) - 1;
	static const uint16_t MaxFeatureObjectId = (1 << 14) - 1;

	RenderOrderConfiguration()
	{
		viewportIndex.SetDefinition(3, sizeof(uint64_t) * 8);
		viewportPass.SetDefinition(3, viewportIndex.shift);
		command.SetDefinition(1, viewportPass.shift);

		// DRAW COMMANDS

		// For transparents

		transparentDepth.SetDefinition(20, command.shift);

		// For opaques

		opaqueDepth.SetDefinition(8, command.shift);
		opaquePadding.SetDefinition(12, opaqueDepth.shift);

		// For all draw commands

		materialId.SetDefinition(11, opaquePadding.shift);

		// For regular mesh component draw commands

		renderObject.SetDefinition(20, materialId.shift);
		meshPart.SetDefinition(3, renderObject.shift);

		// For custom renderer / graphics feature commands

		featureIndex.SetDefinition(7, materialId.shift);
		featureObjectId.SetDefinition(14, featureIndex.shift);

		// CONTROL COMMANDS

		// Type of command
		commandType.SetDefinition(8, command.shift);

		// Command data or offset to buffer, depending on command type
		commandData.SetDefinition(32, commandType.shift);

		// Calculate maximum integer depth values in advance

		maxTransparentDepth = (1ULL << transparentDepth.bits) - 1;
		maxOpaqueDepth = (1ULL << opaqueDepth.bits) - 1;
	}

	BitfieldVariable<uint64_t> viewportIndex;
	BitfieldVariable<uint64_t> viewportPass;
	BitfieldVariable<uint64_t> command;
	BitfieldVariable<uint64_t> transparentDepth;
	BitfieldVariable<uint64_t> opaqueDepth;
	BitfieldVariable<uint64_t> opaquePadding;
	BitfieldVariable<uint64_t> materialId;
	BitfieldVariable<uint64_t> renderObject;
	BitfieldVariable<uint64_t> meshPart;
	BitfieldVariable<uint64_t> featureIndex; // Or custom renderer index
	BitfieldVariable<uint64_t> featureObjectId;
	BitfieldVariable<uint64_t> commandType;
	BitfieldVariable<uint64_t> commandData;

	uint64_t maxTransparentDepth;
	uint64_t maxOpaqueDepth;
};

} // namespace kokko
