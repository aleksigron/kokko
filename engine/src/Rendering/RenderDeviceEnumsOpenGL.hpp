#pragma once

#include <cstdint>

#include "Rendering/RenderTypes.hpp"

namespace kokko
{

uint32_t ConvertDeviceParameter(RenderDeviceParameter parameter);
uint32_t ConvertClipOriginMode(RenderClipOriginMode origin);
uint32_t ConvertClipDepthMode(RenderClipDepthMode depth);
uint32_t ConvertCullFace(RenderCullFace face);
uint32_t ConvertBufferUsage(RenderBufferUsage usage);
uint32_t ConvertBufferTarget(RenderBufferTarget target);
uint32_t ConvertBufferAccess(RenderBufferAccess access);
uint32_t ConvertTextureTarget(RenderTextureTarget target);
uint32_t ConvertTextureDataType(RenderTextureDataType type);
uint32_t ConvertTextureSizedFormat(RenderTextureSizedFormat format);
uint32_t ConvertTextureBaseFormat(RenderTextureBaseFormat format);
uint32_t ConvertTextureParameter(RenderTextureParameter parameter);
uint32_t ConvertTextureFilterMode(RenderTextureFilterMode mode);
uint32_t ConvertTextureWrapMode(RenderTextureWrapMode mode);
uint32_t ConvertTextureCompareMode(RenderTextureCompareMode mode);
uint32_t ConvertDepthCompareFunc(RenderDepthCompareFunc func);
uint32_t ConvertFramebufferTarget(RenderFramebufferTarget target);
uint32_t ConvertFramebufferAttachment(RenderFramebufferAttachment attachment);
uint32_t ConvertBlendFactor(RenderBlendFactor factor);
uint32_t ConvertBlendEquation(RenderBlendEquation equation);
uint32_t ConvertIndexType(RenderIndexType type);
uint32_t ConvertPrimitiveMode(RenderPrimitiveMode mode);
uint32_t ConvertShaderStage(RenderShaderStage stage);
uint32_t ConvertVertexElemType(RenderVertexElemType type);
RenderDebugSource ConvertDebugSource(uint32_t source);
RenderDebugType ConvertDebugType(uint32_t type);
RenderDebugSeverity ConvertDebugSeverity(uint32_t severity);
uint32_t ConvertObjectType(RenderObjectType type);

} // namespace kokko
