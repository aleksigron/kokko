#include "RenderPipeline.hpp"

#include "IncludeOpenGL.hpp"

#include "Color.hpp"

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

void RenderPipeline::DepthTestFunction(unsigned int function)
{
	glDepthFunc(function);
}

void RenderPipeline::DepthTestFunctionNever()
{
	DepthTestFunction(GL_NEVER);
}

void RenderPipeline::DepthTestFunctionLess()
{
	DepthTestFunction(GL_LESS);
}

void RenderPipeline::DepthTestFunctionEqual()
{
	DepthTestFunction(GL_EQUAL);
}

void RenderPipeline::DepthTestFunctionLessEqual()
{
	DepthTestFunction(GL_LEQUAL);
}

void RenderPipeline::DepthTestFunctionGreater()
{
	DepthTestFunction(GL_GREATER);
}

void RenderPipeline::DepthTestFunctionNotEqual()
{
	DepthTestFunction(GL_NOTEQUAL);
}

void RenderPipeline::DepthTestFunctionGreaterEqual()
{
	DepthTestFunction(GL_GEQUAL);
}

void RenderPipeline::DepthTestFunctionAlways()
{
	DepthTestFunction(GL_ALWAYS);
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
