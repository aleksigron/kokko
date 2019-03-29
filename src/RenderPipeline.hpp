#pragma once

struct Color;

namespace RenderPipeline
{
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
};
