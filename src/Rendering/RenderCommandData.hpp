#pragma once

#include <cstdint>

namespace RenderCommandData
{
	struct ClearColorData
	{
		float r;
		float g;
		float b;
		float a;
	};

	struct ViewportData
	{
		int x;
		int y;
		int w;
		int h;
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

	struct AttachFramebufferTexture2D
	{
		unsigned int target;
		unsigned int attachment;
		unsigned int textureTarget;
		unsigned int texture;
		int mipLevel;
	};

	struct BlendFunctionData
	{
		unsigned int srcFactor;
		unsigned int dstFactor;
	};

	struct SetTextureImage2D
	{
		unsigned int target;
		int mipLevel;
		unsigned int internalFormat;
		int width;
		int height;
		unsigned int format;
		unsigned int type;
		const void* data;
	};

	struct SetVertexAttributePointer
	{
		unsigned int attributeIndex;
		int elementCount;
		unsigned int elementType;
		int stride;
		std::uintptr_t offset;
	};
}
