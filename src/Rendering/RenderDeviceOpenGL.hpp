#pragma once

#include "Rendering/RenderDevice.hpp"

class RenderDeviceOpenGL : public RenderDevice
{
	virtual void Clear(unsigned int mask) override;
	virtual void ClearColor(const RenderCommandData::ClearColorData* data) override;
	virtual void ClearDepth(float depth) override;

	virtual void BlendingEnable() override;
	virtual void BlendingDisable() override;
	virtual void BlendFunction(const RenderCommandData::BlendFunctionData* data) override;

	virtual void DepthRange(const RenderCommandData::DepthRangeData* data) override;
	virtual void Viewport(const RenderCommandData::ViewportData* data) override;

	virtual void DepthTestEnable() override;
	virtual void DepthTestDisable() override;

	virtual void DepthTestFunction(unsigned int function) override;

	virtual void DepthWriteEnable() override;
	virtual void DepthWriteDisable() override;

	virtual void CullFaceEnable() override;
	virtual void CullFaceDisable() override;
	virtual void CullFaceFront() override;
	virtual void CullFaceBack() override;

	virtual void BindFramebuffer(const RenderCommandData::BindFramebufferData* data) override;

	virtual void BlitFramebuffer(const RenderCommandData::BlitFramebufferData* data) override;
};
