#pragma once

#include "Math/Vec2.hpp"

#include "Rendering/Framebuffer.hpp"
#include "Rendering/RenderResourceId.hpp"

#include "Resources/MeshId.hpp"

class MeshManager;
class RenderDevice;

namespace kokko
{

class RenderGraphResources
{
public:
	RenderGraphResources(RenderDevice* renderDevice, MeshManager* meshManager);

	void VerifyResourcesAreCreated(Vec2i fullscreenViewportResolution);
	void Deinitialize();

	const Framebuffer& GetGeometryBuffer() const;
	const Framebuffer& GetShadowBuffer() const;
	const Framebuffer& GetLightAccumulationBuffer() const;
	const Framebuffer& GetAmbientOcclusionBuffer() const;

	RenderTextureId GetGeometryBufferAlbedoTexture() const;
	RenderTextureId GetGeometryBufferNormalTexture() const;
	RenderTextureId GetGeometryBufferMaterialTexture() const;

	Vec2i GetFullscreenViewportSize() const;

private:
	static constexpr size_t GbufferAlbedoIndex = 0;
	static constexpr size_t GbufferNormalIndex = 1;
	static constexpr size_t GbufferMaterialIndex = 2;
	static constexpr size_t GbufferColorCount = 3;

	RenderDevice* renderDevice;
	MeshManager* meshManager;

	Framebuffer framebufferGbuffer;
	Framebuffer framebufferShadow;
	Framebuffer framebufferLightAccumulation;
	Framebuffer framebufferAmbientOcclusion;

	Vec2i fullscreenViewportSize;
};

}
