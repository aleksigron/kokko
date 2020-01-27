#pragma once

#include "Core/Color.hpp"
#include "Math/Rectangle.hpp"

namespace RenderCommandData
{
	struct ClearColorData;
	struct BlendFunctionData;
	struct DepthRangeData;
	struct ViewportData;
	struct BindFramebufferData;
	struct BlitFramebufferData;
}

class RenderDevice
{
public:
	virtual void Clear(unsigned int mask) = 0;
	virtual void ClearColor(const RenderCommandData::ClearColorData* data) = 0;
	virtual void ClearDepth(float depth) = 0;

	virtual void BlendingEnable() = 0;
	virtual void BlendingDisable() = 0;
	virtual void BlendFunction(const RenderCommandData::BlendFunctionData* data) = 0;

	virtual void DepthRange(const RenderCommandData::DepthRangeData* data) = 0;
	virtual void Viewport(const RenderCommandData::ViewportData* data) = 0;

	virtual void DepthTestEnable() = 0;
	virtual void DepthTestDisable() = 0;

	virtual void DepthTestFunction(unsigned int function) = 0;

	virtual void DepthWriteEnable() = 0;
	virtual void DepthWriteDisable() = 0;

	virtual void CullFaceEnable() = 0;
	virtual void CullFaceDisable() = 0;
	virtual void CullFaceFront() = 0;
	virtual void CullFaceBack() = 0;

	virtual void BindFramebuffer(const RenderCommandData::BindFramebufferData* data) = 0;

	virtual void BlitFramebuffer(const RenderCommandData::BlitFramebufferData* data) = 0;
};
