#pragma once

#include <cstdint>

#include "Rendering/RenderDeviceEnums.hpp"

namespace RenderCommandData
{
	struct ClearMask
	{
		bool color;
		bool depth;
		bool stencil;
	};

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
		RenderFramebufferAttachment attachment;
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
		RenderTextureSizedFormat format;
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
		RenderTextureBaseFormat format;
		RenderTextureDataType type;
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

	struct SetSamplerParameters
	{
		unsigned int sampler;
		RenderTextureFilterMode minFilter;
		RenderTextureFilterMode magFilter;
		RenderTextureWrapMode wrapModeU;
		RenderTextureWrapMode wrapModeV;
		RenderTextureWrapMode wrapModeW;
		RenderTextureCompareMode compareMode;
		RenderDepthCompareFunc compareFunc;
	};

	struct SetVertexAttributePointer
	{
		unsigned int attributeIndex;
		int elementCount;
		RenderVertexElemType elementType;
		int stride;
		std::uintptr_t offset;
	};

	struct BindBufferRange
	{
		RenderBufferTarget target;
		unsigned int bindingPoint;
		unsigned int buffer;
		intptr_t offset;
		size_t length;
	};

	struct SetBufferStorage
	{
		RenderBufferTarget target;
		size_t size;
		const void* data;
		bool dynamicStorage; // Client can update data after creation using SetBufferSubData
		bool mapReadAccess; // Client can map data for read access
		bool mapWriteAccess; // Client can map data for write access
		bool mapPersistent; // Client can use the buffer for other commands while it is mapped
		bool mapCoherent; // While persistently mapped, changes to data are coherent
	};

	struct MapBufferRange
	{
		RenderBufferTarget target;
		intptr_t offset;
		size_t length;
		bool readAccess; // Data can be read
		bool writeAccess; // Data can be written
		bool invalidateRange; // Previous contents of range can be discarded
		bool invalidateBuffer; // Previous contents of buffer can be discarded
		bool flushExplicit; // Changes are not implicitly flushed by unmap
		bool unsynchronized; // No synchronization of other operations is attempted while mapped
		bool persistent; // Mapping is to be made in a persistent fashion
		bool coherent; // Persistent mapping is also to be coherent
	};
}
