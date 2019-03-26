#pragma once

#include "SceneLayer.hpp"
#include "TransparencyType.hpp"
#include "RenderOrder.hpp"
#include "MaterialData.hpp"

struct Color;

class RenderPipeline
{
private:
	RenderOrderConfiguration renderOrder;

public:
	bool ParseControlCommand(uint64_t orderKey);

	uint64_t CreateControlCommand(
		SceneLayer layer,
		TransparencyType transparency,
		RenderOrder::ControlCommandType command)
	{
		uint64_t c = 0;

		renderOrder.viewportIndex.AssignValue(c, RenderOrder::Viewport_Fullscreen);
		renderOrder.viewportLayer.AssignValue(c, static_cast<uint64_t>(layer));
		renderOrder.transparencyType.AssignValue(c, static_cast<uint64_t>(transparency));
		renderOrder.command.AssignValue(c, RenderOrder::Command_Control);
		renderOrder.commandType.AssignValue(c, command);

		return c;
	}

	uint64_t CreateDrawCommand(
		SceneLayer layer,
		TransparencyType transparency,
		float depth,
		MaterialId material,
		unsigned int renderObjectId)
	{
		depth = (depth > 1.0f ? 1.0f : (depth < 0.0f ? 0.0f : depth));

		uint64_t c = 0;

		renderOrder.viewportIndex.AssignValue(c, RenderOrder::Viewport_Fullscreen);
		renderOrder.viewportLayer.AssignValue(c, static_cast<uint64_t>(layer));
		renderOrder.transparencyType.AssignValue(c, static_cast<uint64_t>(transparency));
		renderOrder.command.AssignValue(c, RenderOrder::Command_Draw);

		if ((0xfe & static_cast<unsigned char>(transparency)) != 0)
		{
			uint64_t intDepth(renderOrder.maxTransparentDepth * (1.0f - depth));
			renderOrder.transparentDepth.AssignValue(c, intDepth);
		}
		else
		{
			uint64_t intDepth(renderOrder.maxOpaqueDepth * depth);
			renderOrder.opaqueDepth.AssignValue(c, intDepth);
		}

		renderOrder.materialId.AssignValue(c, material.i);
		renderOrder.renderObject.AssignValue(c, renderObjectId);

		return c;
	}

	static void ClearColorAndDepth(const Color& color);

	static void BlendingEnable();
	static void BlendingDisable();

	static void DepthTestEnable();
	static void DepthTestDisable();
	static void DepthTestFunctionLess();

	static void DepthWriteEnable();
	static void DepthWriteDisable();

	static void CullFaceEnable();
	static void CullFaceDisable();
	static void CullFaceFront();
	static void CullFaceBack();
};
