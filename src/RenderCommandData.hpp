#pragma once

namespace RenderCommandData
{
	struct ClearColorData
	{
		float r;
		float g;
		float b;
		float a;
	};

	struct DepthRangeData
	{
		float near;
		float far;
	};

	struct BindFramebufferData
	{
		unsigned int target;
		unsigned int framebuffer;
	};

	struct BlitFramebufferData
	{
		float srcLeft;
		float srcTop;
		float srcWidth;
		float srcHeight;

		float dstLeft;
		float dstTop;
		float dstWidth;
		float dstHeight;

		unsigned int mask;
		unsigned int filter;
	};
}
