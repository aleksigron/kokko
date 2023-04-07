#pragma once

#include <cstddef>
#include <cstdint>

#include "Core/Array.hpp"
#include "Core/BitPack.hpp"
#include "Core/HashMap.hpp"
#include "Core/Optional.hpp"
#include "Core/Range.hpp"

#include "Math/Mat4x4.hpp"

#include "Rendering/Framebuffer.hpp"
#include "Rendering/Light.hpp"
#include "Rendering/RendererCommandList.hpp"
#include "Rendering/RenderOrder.hpp"

#include "Resources/MaterialData.hpp"

class Allocator;
class CameraSystem;
class LightManager;
class ShaderManager;
class MeshManager;
class MaterialManager;
class TextureManager;
class RenderDevice;
class Scene;
class DebugVectorRenderer;
class PostProcessRenderer;
class RenderTargetContainer;
class Framebuffer;

struct CameraParameters;
struct RenderViewport;

namespace kokko
{

class EnvironmentSystem;
class GraphicsFeature;
class MeshComponentSystem;
class RenderDebugSettings;
class RenderGraphResources;
class UniformData;
class Window;

struct ResourceManagers;

namespace render
{
class CommandEncoder;
}

class Renderer
{
private:
	static const unsigned int FramesInFlightCount = 1;
	static const unsigned int MaxViewportCount = 8;
	static const unsigned int MaxFramebufferCount = 4;
	static const unsigned int MaxFramebufferTextureCount = 16;

	static const size_t ObjectUniformBufferSize = 512 * 1024;

	Allocator* allocator;
	RenderDevice* device;
	render::CommandEncoder* encoder;
	MeshComponentSystem* componentSystem;

	RenderGraphResources* renderGraphResources;
	RenderTargetContainer* renderTargetContainer;
	PostProcessRenderer* postProcessRenderer;
	
	RenderFramebufferId targetFramebufferId;

	RenderViewport* viewportData;
	unsigned int viewportCount;
	unsigned int viewportIndexFullscreen;
	Range<unsigned int> viewportIndicesShadowCascade;

	MaterialId shadowMaterial;
	MaterialId fallbackMeshMaterial;

	Array<uint8_t> uniformStagingBuffer;
	Array<RenderBufferId> objectUniformBufferLists[FramesInFlightCount];
	unsigned int currentFrameIndex;

	intptr_t objectUniformBlockStride;
	intptr_t objectsPerUniformBuffer;

	RenderOrderConfiguration renderOrder;

	Scene* scene;
	CameraSystem* cameraSystem;
	LightManager* lightManager;
	kokko::EnvironmentSystem* environmentSystem;
	ShaderManager* shaderManager;
	MeshManager* meshManager;
	MaterialManager* materialManager;
	TextureManager* textureManager;

	bool lockCullingCamera;
	Mat4x4fBijection lockCullingCameraTransform;

	RendererCommandList commandList;
	Array<BitPack> objectVisibility;

	Array<LightId> lightResultArray;

	Array<kokko::GraphicsFeature*> graphicsFeatures;

	RenderBufferId normalDebugBufferId;

	void BindMaterialTextures(const kokko::UniformData& materialUniforms) const;

	CameraParameters GetCameraParameters(const Optional<CameraParameters>& editorCamera,
		const Framebuffer& targetFramebuffer);

	// Returns the number of object draw commands added
	unsigned int PopulateCommandList(const Optional<CameraParameters>& editorCamera,
		const Framebuffer& targetFramebuffer);

	void UpdateUniformBuffers(size_t objectDrawCount);

	bool IsDrawCommand(uint64_t orderKey);
	bool ParseControlCommand(uint64_t orderKey);

public:
	Renderer(Allocator* allocator,
		RenderDevice* renderDevice,
		kokko::MeshComponentSystem* componentSystem,
		Scene* scene,
		CameraSystem* cameraSystem,
		LightManager* lightManager,
		kokko::EnvironmentSystem* environmentSystem,
		const kokko::ResourceManagers& resourceManagers);
	~Renderer();

	void Initialize();
	void Deinitialize();

	void SetLockCullingCamera(bool lockEnable);
	const Mat4x4f& GetCullingCameraTransform() const;

	void Render(kokko::Window* window, const Optional<CameraParameters>& editorCamera, const Framebuffer& targetFramebuffer);

	void DebugRender(DebugVectorRenderer* vectorRenderer, const kokko::RenderDebugSettings& settings);

	// Transfers ownership to Renderer
	void AddGraphicsFeature(kokko::GraphicsFeature* graphicsFeature);
	void RemoveGraphicsFeature(kokko::GraphicsFeature* graphicsFeature);
};

} // namespace kokko
