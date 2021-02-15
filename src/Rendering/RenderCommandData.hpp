#pragma once

#include <cstdint>

#include "Rendering/RenderDeviceEnums.hpp"

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
		RenderFramebufferTarget target;
		unsigned int framebuffer;
	};

	struct AttachFramebufferTexture2D
	{
		RenderFramebufferTarget target;
		unsigned int attachment;
		RenderTextureTarget textureTarget;
		unsigned int texture;
		int mipLevel;
	};

	struct BlendFunctionData
	{
		RenderBlendFactor srcFactor;
		RenderBlendFactor dstFactor;
	};

	struct SetTextureStorage2D
	{
		RenderTextureTarget target;
		int levels;
		unsigned int internalFormat;
		int width;
		int height;
	};

	struct SetTextureImage2D
	{
		RenderTextureTarget target;
		int mipLevel;
		unsigned int internalFormat;
		int width;
		int height;
		unsigned int format;
		unsigned int type;
		const void* data;
	};

	struct SetTextureSubImage2D
	{
		RenderTextureTarget target;
		int mipLevel;
		int xOffset;
		int yOffset;
		int width;
		int height;
		unsigned int format;
		unsigned int type;
		const void* data;
	};

	struct SetTextureImageCompressed2D
	{
		RenderTextureTarget target;
		int mipLevel;
		unsigned int internalFormat;
		int width;
		int height;
		unsigned int dataSize;
		const void* data;
	};

	struct SetVertexAttributePointer
	{
		unsigned int attributeIndex;
		int elementCount;
		RenderVertexElemType elementType;
		int stride;
		std::uintptr_t offset;
	};
}
