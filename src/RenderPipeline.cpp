#include "RenderPipeline.hpp"

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

RenderPipeline::RenderPipeline()
{
	this->InitializeRenderOrder();
}

RenderPipeline::~RenderPipeline()
{
}

void RenderPipeline::InitializeRenderOrder()
{
	RenderOrderConfiguration& conf = orderConfiguration;

	conf.viewportIndex.SetDefinition(4, sizeof(uint64_t) * 8);
	conf.viewportLayer.SetDefinition(4, conf.viewportIndex.shift);
	conf.transparencyType.SetDefinition(5, conf.viewportLayer.shift);
	conf.command.SetDefinition(1, conf.transparencyType.shift);

	conf.transparentDepth.SetDefinition(24, conf.command.shift);
	conf.transparentMaterialId.SetDefinition(16, conf.transparentDepth.shift);

	conf.opaqueDepth.SetDefinition(8, conf.command.shift);
	conf.opaqueMaterialId.SetDefinition(16, conf.opaqueDepth.shift);

	conf.commandType.SetDefinition(8, conf.command.shift);
	conf.commandData.SetDefinition(32, conf.commandType.shift);
}

void RenderPipeline::ParseControlCommand(uint64_t orderKey)
{
	using namespace RenderOrder;

	uint64_t commandTypeInt = orderConfiguration.commandType.GetValue(orderKey);
	ControlCommandType command = static_cast<ControlCommandType>(commandTypeInt);

	switch (command)
	{
		case Control_GlEnable:
			break;

		case Control_GlDisable:
			break;

		case Control_BlendingEnable:
			this->BlendingEnable();
			break;

		case Control_BlendingDisable:
			this->BlendingDisable();
			break;

		case Control_DepthTestEnable:
			this->DepthTestEnable();
			break;

		case Control_DepthTestDisable:
			this->DepthTestDisable();
			break;

		case Control_DepthWriteEnable:
			this->DepthWriteEnable();
			break;

		case Control_DepthWriteDisable:
			this->DepthWriteDisable();
			break;
	}
}

uint64_t RenderPipeline::CreateControlCommand(SceneLayer layer,
											  TransparencyType transparency,
											  RenderOrder::ControlCommandType command)
{
	uint64_t c = 0;

	orderConfiguration.viewportIndex.AssignValue(c, RenderOrder::Viewport_Fullscreen);
	orderConfiguration.viewportLayer.AssignValue(c, static_cast<uint64_t>(layer));
	orderConfiguration.transparencyType.AssignValue(c, static_cast<uint64_t>(transparency));
	orderConfiguration.command.AssignValue(c, RenderOrder::Command_Control);
	orderConfiguration.commandType.AssignValue(c, command);

	return c;
}

uint64_t RenderPipeline::CreateDrawCommand(SceneLayer layer, TransparencyType transparency,
										   float depth, unsigned int materialId)
{
	if (depth > 1.0f)
		depth = 1.0f;
	else if (depth < 0.0f)
		depth = 0.0f;

	uint64_t c = 0;

	orderConfiguration.viewportIndex.AssignValue(c, RenderOrder::Viewport_Fullscreen);
	orderConfiguration.viewportLayer.AssignValue(c, static_cast<uint64_t>(layer));
	orderConfiguration.transparencyType.AssignValue(c, static_cast<uint64_t>(transparency));
	orderConfiguration.command.AssignValue(c, RenderOrder::Command_Draw);

	switch (transparency)
	{
		case TransparencyType::Opaque:
		case TransparencyType::AlphaTest:
		{
			float scaledDepth = ((1 << orderConfiguration.opaqueDepth.bits) - 1) * depth;
			uint64_t intDepth = static_cast<uint64_t>(scaledDepth);

			orderConfiguration.opaqueDepth.AssignValue(c, intDepth);
			orderConfiguration.opaqueMaterialId.AssignValue(c, materialId);
		}
			break;

		case TransparencyType::TransparentMix:
		case TransparencyType::TransparentAdd:
		case TransparencyType::TransparentSub:
		{
			float scaledDepth = ((1 << orderConfiguration.transparentDepth.bits) - 1) * depth;
			uint64_t intDepth = static_cast<uint64_t>(scaledDepth);

			orderConfiguration.transparentDepth.AssignValue(c, intDepth);
			orderConfiguration.transparentMaterialId.AssignValue(c, materialId);
		}
			break;
	}

	return c;
}

void RenderPipeline::BlendingEnable()
{
	glEnable(GL_BLEND);
}

void RenderPipeline::BlendingDisable()
{
	glDisable(GL_BLEND);
}

void RenderPipeline::DepthTestEnable()
{
	glEnable(GL_DEPTH_TEST);
}

void RenderPipeline::DepthTestDisable()
{
	glDisable(GL_DEPTH_TEST);
}

void RenderPipeline::DepthTestFunctionLess()
{
	glDepthFunc(GL_LESS);
}

void RenderPipeline::DepthWriteEnable()
{
	glDepthMask(GL_TRUE);
}

void RenderPipeline::DepthWriteDisable()
{
	glDepthMask(GL_FALSE);
}

void RenderPipeline::CullFaceEnable()
{
	glEnable(GL_CULL_FACE);
}

void RenderPipeline::CullFaceDisable()
{
	glDisable(GL_CULL_FACE);
}

void RenderPipeline::CullFaceFront()
{
	glCullFace(GL_FRONT);
}

void RenderPipeline::CullFaceBack()
{
	glCullFace(GL_BACK);
}
