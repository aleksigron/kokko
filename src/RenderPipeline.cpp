#include "RenderPipeline.hpp"

#include "IncludeOpenGL.hpp"

#include "Color.hpp"

bool RenderPipeline::ParseControlCommand(uint64_t orderKey)
{
	using namespace RenderOrder;

	if (renderOrder.command.GetValue(orderKey) == Command_Control)
	{
		uint64_t commandTypeInt = renderOrder.commandType.GetValue(orderKey);
		ControlCommandType command = static_cast<ControlCommandType>(commandTypeInt);

		switch (command)
		{
			case Control_GlEnable:
				break;

			case Control_GlDisable:
				break;

			case Control_BlendingEnable:
				RenderPipeline::BlendingEnable();
				break;

			case Control_BlendingDisable:
				RenderPipeline::BlendingDisable();
				break;

			case Control_DepthTestEnable:
				RenderPipeline::DepthTestEnable();
				break;

			case Control_DepthTestDisable:
				RenderPipeline::DepthTestDisable();
				break;

			case Control_DepthWriteEnable:
				RenderPipeline::DepthWriteEnable();
				break;

			case Control_DepthWriteDisable:
				RenderPipeline::DepthWriteDisable();
				break;
		}

		return true;
	}
	else
		return false;
}

void RenderPipeline::ClearColorAndDepth(const Color& color)
{
	glClearColor(color.r, color.g, color.b, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
