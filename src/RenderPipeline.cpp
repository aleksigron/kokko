#include "RenderPipeline.hpp"

#include "IncludeOpenGL.hpp"

#include "RenderCommandData.hpp"

void RenderPipeline::Clear(unsigned int mask)
{
	glClear(mask);
}

void RenderPipeline::ClearColor(const RenderCommandData::ClearColorData* data)
{
	glClearColor(data->r, data->g, data->b, data->a);
}

void RenderPipeline::ClearDepth(float depth)
{
	glClearDepth(depth);
}

void RenderPipeline::BlendingEnable()
{
	glEnable(GL_BLEND);
}

void RenderPipeline::BlendingDisable()
{
	glDisable(GL_BLEND);
}

void RenderPipeline::DepthRange(const RenderCommandData::DepthRangeData* data)
{
	glDepthRange(data->near, data->far);
}

void RenderPipeline::Viewport(const RenderCommandData::ViewportData* data)
{
	glViewport(data->x, data->y, data->w, data->h);
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

void RenderPipeline::BindFramebuffer(const RenderCommandData::BindFramebufferData* data)
{
	glBindFramebuffer(data->target, data->framebuffer);
}

void RenderPipeline::BlitFramebuffer(const RenderCommandData::BlitFramebufferData* data)
{
	glBlitFramebuffer(
		data->srcLeft, data->srcTop, data->srcWidth, data->srcHeight,
		data->dstLeft, data->dstTop, data->dstWidth, data->dstHeight,
		data->mask, data->filter);
}
