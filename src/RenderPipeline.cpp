#include "RenderPipeline.hpp"

#include "IncludeOpenGL.hpp"

RenderPipeline::RenderPipeline()
{
	this->InitializeRenderOrder();
}

RenderPipeline::~RenderPipeline()
{
}

void RenderPipeline::InitializeRenderOrder()
{
	RenderOrderConfiguration& conf = this->orderConfig;

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

bool RenderPipeline::ParseControlCommand(uint64_t orderKey)
{
	if (orderConfig.command.GetValue(orderKey) == RenderOrder::Command_Control)
	{
		using namespace RenderOrder;

		uint64_t commandTypeInt = orderConfig.commandType.GetValue(orderKey);
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

		return true;
	}
	else
		return false;
}

uint64_t RenderPipeline::CreateControlCommand(SceneLayer layer,
											  TransparencyType transparency,
											  RenderOrder::ControlCommandType command)
{
	uint64_t c = 0;

	orderConfig.viewportIndex.AssignValue(c, RenderOrder::Viewport_Fullscreen);
	orderConfig.viewportLayer.AssignValue(c, static_cast<uint64_t>(layer));
	orderConfig.transparencyType.AssignValue(c, static_cast<uint64_t>(transparency));
	orderConfig.command.AssignValue(c, RenderOrder::Command_Control);
	orderConfig.commandType.AssignValue(c, command);

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

	orderConfig.viewportIndex.AssignValue(c, RenderOrder::Viewport_Fullscreen);
	orderConfig.viewportLayer.AssignValue(c, static_cast<uint64_t>(layer));
	orderConfig.transparencyType.AssignValue(c, static_cast<uint64_t>(transparency));
	orderConfig.command.AssignValue(c, RenderOrder::Command_Draw);

	switch (transparency)
	{
		case TransparencyType::Opaque:
		case TransparencyType::AlphaTest:
		{
			uint64_t depthMaxValue = (1 << orderConfig.opaqueDepth.bits) - 1;
			uint64_t intDepth = static_cast<uint64_t>(depthMaxValue * depth);

			orderConfig.opaqueDepth.AssignValue(c, intDepth);
			orderConfig.opaqueMaterialId.AssignValue(c, materialId);
		}
			break;

		case TransparencyType::TransparentMix:
		{
			uint64_t depthMaxValue = (1 << orderConfig.transparentDepth.bits) - 1;
			uint64_t intDepth = static_cast<uint64_t>(depthMaxValue * (1.0f - depth));

			orderConfig.transparentDepth.AssignValue(c, intDepth);
			orderConfig.transparentMaterialId.AssignValue(c, materialId);
		}
			break;

		case TransparencyType::TransparentAdd:
		case TransparencyType::TransparentSub:
			orderConfig.transparentDepth.AssignValue(c, 0);
			orderConfig.transparentMaterialId.AssignValue(c, materialId);
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
