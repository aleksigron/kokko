#pragma once

#include "Color.hpp"
#include "Rectangle.hpp"

namespace RenderCommandData
{
	struct ClearData;
	struct BindFramebufferData;
	struct BlitFramebufferData;
}

namespace RenderPipeline
{
	void Clear(const RenderCommandData::ClearData* data);
	void ClearColorAndDepth(const Color& color);

	void BlendingEnable();
	void BlendingDisable();

	void DepthTestEnable();
	void DepthTestDisable();

	void DepthTestFunction(unsigned int function);
	void DepthTestFunctionNever();
	void DepthTestFunctionLess();
	void DepthTestFunctionEqual();
	void DepthTestFunctionLessEqual();
	void DepthTestFunctionGreater();
	void DepthTestFunctionNotEqual();
	void DepthTestFunctionGreaterEqual();
	void DepthTestFunctionAlways();

	void DepthWriteEnable();
	void DepthWriteDisable();

	void CullFaceEnable();
	void CullFaceDisable();
	void CullFaceFront();
	void CullFaceBack();

	void BindFramebuffer(const RenderCommandData::BindFramebufferData* data);

	void BlitFramebuffer(const RenderCommandData::BlitFramebufferData* data);
};
