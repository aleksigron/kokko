#pragma once

namespace RenderCommandData
{
	struct ClearData
	{
		float r;
		float g;
		float b;
		float a;
		unsigned int mask;
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
