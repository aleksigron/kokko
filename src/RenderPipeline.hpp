#pragma once

#include "Color.hpp"
#include "Math/Rectangle.hpp"

namespace RenderCommandData
{
	struct ClearColorData;
	struct DepthRangeData;
	struct ViewportData;
	struct BindFramebufferData;
	struct BlitFramebufferData;
}

namespace RenderPipeline
{
	void Clear(unsigned int mask);
	void ClearColor(const RenderCommandData::ClearColorData* data);
	void ClearDepth(float depth);

	void BlendingEnable();
	void BlendingDisable();

	void DepthRange(const RenderCommandData::DepthRangeData* data);
	void Viewport(const RenderCommandData::ViewportData* data);

	void DepthTestEnable();
	void DepthTestDisable();

	void DepthTestFunction(unsigned int function);

	void DepthWriteEnable();
	void DepthWriteDisable();

	void CullFaceEnable();
	void CullFaceDisable();
	void CullFaceFront();
	void CullFaceBack();

	void BindFramebuffer(const RenderCommandData::BindFramebufferData* data);

	void BlitFramebuffer(const RenderCommandData::BlitFramebufferData* data);
};
