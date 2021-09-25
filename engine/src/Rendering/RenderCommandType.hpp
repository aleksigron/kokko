#pragma once

enum class RenderPass
{
	OpaqueGeometry = 0, // Opaque + AlphaTest
	OpaqueLighting = 2,
	Skybox = 3,
	Transparent = 4, // TransparentMix + TransparentAdd + TransparentSub
	PostProcess = 7
};

enum class RenderCommandType
{
	Control = 0,
	Draw = 1
};

enum class RenderControlType
{
	BlendingEnable,
	BlendingDisable,
	
	// Data: srcFactor, dstFactor
	BlendFunction,

	Viewport, // Data: x, y, w, h

	ScissorTestEnable,
	ScissorTestDisable,

	DepthRange, // Data: near, far

	DepthTestEnable,
	DepthTestDisable,
	DepthTestFunction, // Data: function enum

	DepthWriteEnable,
	DepthWriteDisable,

	CullFaceEnable,
	CullFaceDisable,
	CullFaceFront,
	CullFaceBack,

	Clear, // Data: color, depth, stencil
	ClearColor, // Data: r, g, b, a
	ClearDepth, // Data: depth

	// Data: target, framebuffer ID
	BindFramebuffer,

	FramebufferSrgbEnable,
	FramebufferSrgbDisable,
};
