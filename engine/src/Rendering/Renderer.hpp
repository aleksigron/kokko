#pragma once

#include <cstddef>
#include <cstdint>

#include "Core/Array.hpp"
#include "Core/BitPack.hpp"
#include "Core/HashMap.hpp"
#include "Core/Optional.hpp"
#include "Core/Range.hpp"
#include "Core/UniquePtr.hpp"

#include "Math/Mat4x4.hpp"

#include "Rendering/Framebuffer.hpp"
#include "Rendering/Light.hpp"
#include "Rendering/RendererCommandList.hpp"
#include "Rendering/RenderOrder.hpp"

#include "Resources/MaterialData.hpp"

namespace kokko
{

class Allocator;
class CameraSystem;
class DebugVectorRenderer;
class EnvironmentSystem;
class GraphicsFeature;
class Framebuffer;
class LightManager;
class MaterialManager;
class MeshComponentSystem;
class ModelManager;
class PostProcessRenderer;
class RenderDebugSettings;
class RenderGraphResources;
class RenderTargetContainer;
class Scene;
class ShaderManager;
class TextureManager;
class UniformData;
class Window;

struct CameraParameters;
struct RenderViewport;
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
	kokko::render::Device* device;
	render::CommandEncoder* encoder;
	MeshComponentSystem* componentSystem;

	UniquePtr<RenderGraphResources> renderGraphResources;
	UniquePtr<RenderTargetContainer> renderTargetContainer;
	UniquePtr<PostProcessRenderer> postProcessRenderer;
	
	render::FramebufferId targetFramebufferId;

	RenderViewport* viewportData;
	unsigned int viewportCount;
	unsigned int viewportIndexFullscreen;
	Range<unsigned int> viewportIndicesShadowCascade;

	MaterialId shadowMaterial;
	MaterialId fallbackMeshMaterial;

	Array<uint8_t> uniformStagingBuffer;
	Array<render::BufferId> objectUniformBufferLists[FramesInFlightCount];
	unsigned int currentFrameIndex;

	intptr_t objectUniformBlockStride;
	intptr_t objectsPerUniformBuffer;

	RenderOrderConfiguration renderOrder;

	Scene* scene;
	CameraSystem* cameraSystem;
	LightManager* lightManager;
	EnvironmentSystem* environmentSystem;
	ShaderManager* shaderManager;
	ModelManager* modelManager;
	MaterialManager* materialManager;
	TextureManager* textureManager;
	const RenderDebugSettings* renderDebug;

	bool lockCullingCamera;
	Mat4x4fBijection lockCullingCameraTransform;

	RendererCommandList commandList;
	Array<BitPack> objectVisibility;

	Array<LightId> lightResultArray;

	Array<GraphicsFeature*> graphicsFeatures;

	render::BufferId normalDebugBufferId;

	void BindMaterialTextures(const kokko::UniformData& materialUniforms) const;

	CameraParameters GetCameraParameters(const Optional<CameraParameters>& editorCamera,
		const render::Framebuffer& targetFramebuffer);

	// Returns the number of object draw commands added
	unsigned int PopulateCommandList(const Optional<CameraParameters>& editorCamera,
		const render::Framebuffer& targetFramebuffer);

	void UpdateUniformBuffers(size_t objectDrawCount);

	bool IsDrawCommand(uint64_t orderKey);
	bool ParseControlCommand(uint64_t orderKey);

public:
	Renderer(Allocator* allocator,
		render::Device* renderDevice,
		render::CommandEncoder* commandEncoder,
		MeshComponentSystem* componentSystem,
		Scene* scene,
		CameraSystem* cameraSystem,
		LightManager* lightManager,
		EnvironmentSystem* environmentSystem,
		const ResourceManagers& resourceManagers,
		const RenderDebugSettings* renderDebug);
	~Renderer();

	void Initialize();
	void Deinitialize();

	void SetLockCullingCamera(bool lockEnable);
	const Mat4x4f& GetCullingCameraTransform() const;

	void Render(Window* window, const Optional<CameraParameters>& editorCamera, const render::Framebuffer& targetFramebuffer);

	void DebugRender(DebugVectorRenderer* vectorRenderer);

	// Transfers ownership to Renderer
	void AddGraphicsFeature(kokko::GraphicsFeature* graphicsFeature);
	void RemoveGraphicsFeature(kokko::GraphicsFeature* graphicsFeature);
};

} // namespace kokko
