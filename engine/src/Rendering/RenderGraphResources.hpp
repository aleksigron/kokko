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
	RenderGraphResources(kokko::render::Device* renderDevice, MeshManager* meshManager);

	void VerifyResourcesAreCreated(Vec2i fullscreenViewportResolution);
	void Deinitialize();

	const render::Framebuffer& GetGeometryBuffer() const;
	const render::Framebuffer& GetShadowBuffer() const;
	const render::Framebuffer& GetLightAccumulationBuffer() const;
	const render::Framebuffer& GetAmbientOcclusionBuffer() const;

	render::TextureId GetGeometryBufferAlbedoTexture() const;
	render::TextureId GetGeometryBufferNormalTexture() const;
	render::TextureId GetGeometryBufferMaterialTexture() const;

	Vec2i GetFullscreenViewportSize() const;

private:
	static constexpr size_t GbufferAlbedoIndex = 0;
	static constexpr size_t GbufferNormalIndex = 1;
	static constexpr size_t GbufferMaterialIndex = 2;
	static constexpr size_t GbufferColorCount = 3;

	kokko::render::Device* renderDevice;
	MeshManager* meshManager;

	render::Framebuffer framebufferGbuffer;
	render::Framebuffer framebufferShadow;
	render::Framebuffer framebufferLightAccumulation;
	render::Framebuffer framebufferAmbientOcclusion;

	Vec2i fullscreenViewportSize;
};

} // namespace kokko
