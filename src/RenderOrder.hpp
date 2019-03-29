#pragma once

#include <cstdint>

#include "SceneLayer.hpp"
#include "MaterialData.hpp"
#include "TransparencyType.hpp"
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
		Control_BlendingEnable,
		Control_BlendingDisable,

		Control_DepthTestEnable,
		Control_DepthTestDisable,
		Control_DepthTestFunction,

		Control_DepthWriteEnable,
		Control_DepthWriteDisable,

		Control_CullFaceEnable,
		Control_CullFaceDisable,
		Control_CullFaceFront,
		Control_CullFaceBack
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

	uint64_t Control(
		SceneLayer layer,
		TransparencyType transparency,
		RenderOrder::ControlCommandType type,
		unsigned int order = 0,
		unsigned int data = 0)
	{
		uint64_t c = 0;

		viewportIndex.AssignValue(c, RenderOrder::Viewport_Fullscreen);
		viewportLayer.AssignValue(c, static_cast<uint64_t>(layer));
		transparencyType.AssignValue(c, static_cast<uint64_t>(transparency));
		command.AssignValue(c, RenderOrder::Command_Control);
		commandOrder.AssignValue(c, order);
		commandType.AssignValue(c, type);
		commandData.AssignValue(c, data);

		return c;
	}

	uint64_t Draw(
		SceneLayer layer,
		TransparencyType transparency,
		float depth,
		MaterialId material,
		unsigned int renderObjectId)
	{
		depth = (depth > 1.0f ? 1.0f : (depth < 0.0f ? 0.0f : depth));

		uint64_t c = 0;

		viewportIndex.AssignValue(c, RenderOrder::Viewport_Fullscreen);
		viewportLayer.AssignValue(c, static_cast<uint64_t>(layer));
		transparencyType.AssignValue(c, static_cast<uint64_t>(transparency));
		command.AssignValue(c, RenderOrder::Command_Draw);

		if ((0xfe & static_cast<unsigned char>(transparency)) != 0)
		{
			uint64_t intDepth(maxTransparentDepth * (1.0f - depth));
			transparentDepth.AssignValue(c, intDepth);
		}
		else
		{
			uint64_t intDepth(maxOpaqueDepth * depth);
			opaqueDepth.AssignValue(c, intDepth);
		}

		materialId.AssignValue(c, material.i);
		renderObject.AssignValue(c, renderObjectId);

		return c;
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
	BitfieldVariable<uint64_t> commandOrder;
	BitfieldVariable<uint64_t> commandType;
	BitfieldVariable<uint64_t> commandData;

	uint64_t maxTransparentDepth;
	uint64_t maxOpaqueDepth;
};
