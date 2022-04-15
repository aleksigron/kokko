#pragma once

#include <cstddef>
#include <cstdint>

#include "Core/Array.hpp"
#include "Core/BitPack.hpp"
#include "Core/HashMap.hpp"
#include "Core/Optional.hpp"

#include "Rendering/CustomRenderer.hpp"
#include "Rendering/Framebuffer.hpp"
#include "Rendering/Light.hpp"
#include "Rendering/RenderCommandList.hpp"
#include "Rendering/RenderOrder.hpp"

#include "Resources/MaterialData.hpp"
#include "Resources/MeshId.hpp"
#include "Resources/ShaderId.hpp"

class Allocator;
class CameraSystem;
class LightManager;
class ShaderManager;
class MeshManager;
class MaterialManager;
class TextureManager;
class EntityManager;
class RenderDevice;
class Scene;
class DebugVectorRenderer;
class CustomRenderer;
class ScreenSpaceAmbientOcclusion;
class BloomEffect;
class PostProcessRenderer;
class RenderTargetContainer;
class Framebuffer;

struct BoundingBox;
struct CameraParameters;
struct RenderViewport;
struct ShaderData;
struct ProjectionParameters;
struct LightingUniformBlock;
struct PostProcessRenderPass;

namespace kokko
{

class EnvironmentSystem;
class MeshComponentSystem;
class RenderDebugSettings;
class UniformData;

struct ResourceManagers;

}

class Renderer : public CustomRenderer
{
private:
	static const unsigned int FramesInFlightCount = 1;
	static const unsigned int MaxViewportCount = 8;
	static const unsigned int MaxFramebufferCount = 4;
	static const unsigned int MaxFramebufferTextureCount = 16;

	static const size_t GbufferAlbedoIndex = 0;
	static const size_t GbufferNormalIndex = 1;
	static const size_t GbufferMaterialIndex = 2;
	static const size_t GbufferColorCount = 3;

	static const size_t ObjectUniformBufferSize = 512 * 1024;

	Allocator* allocator;
	RenderDevice* device;
	kokko::MeshComponentSystem* componentSystem;

	RenderTargetContainer* renderTargetContainer;
	PostProcessRenderer* postProcessRenderer;

	ScreenSpaceAmbientOcclusion* ssao;
	BloomEffect* bloomEffect;

	Framebuffer framebufferShadow;
	Framebuffer framebufferGbuffer;
	Framebuffer framebufferLightAcc;

	unsigned int targetFramebufferId;

	RenderViewport* viewportData;
	unsigned int viewportCount;
	unsigned int viewportIndexFullscreen;

	MeshId fullscreenMesh;
	ShaderId lightingShaderId;
	ShaderId tonemappingShaderId;
	MaterialId shadowMaterial;
	MaterialId fallbackMeshMaterial;
	unsigned int lightingUniformBufferId;
	unsigned int tonemapUniformBufferId;

	unsigned int brdfLutTextureId;

	Array<unsigned char> uniformStagingBuffer;
	Array<unsigned int> objectUniformBufferLists[FramesInFlightCount];
	unsigned int currentFrameIndex;

	intptr_t objectUniformBlockStride;
	intptr_t objectsPerUniformBuffer;

	unsigned int deferredLightingCallback;
	unsigned int skyboxRenderCallback;
	unsigned int postProcessCallback;

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

	RenderCommandList commandList;
	Array<BitPack> objectVisibility;

	Array<LightId> lightResultArray;

	Array<CustomRenderer*> customRenderers;

	ShaderId skyboxShaderId;
	MeshId skyboxMeshId;
	unsigned int skyboxUniformBufferId;
	unsigned int normalDebugBufferId;

	void CreateResolutionDependentFramebuffers(int width, int height);
	void DestroyResolutionDependentFramebuffers();

	void BindMaterialTextures(const kokko::UniformData& materialUniforms) const;
	void BindTextures(const ShaderData& shader, unsigned int count,
		const uint32_t* nameHashes, const unsigned int* textures);

	void UpdateLightingDataToUniformBuffer(
		const ProjectionParameters& projection, LightingUniformBlock& uniformsOut);

	CameraParameters GetCameraParameters(const Optional<CameraParameters>& editorCamera,
		const Framebuffer& targetFramebuffer);

	// Returns the number of object draw commands added
	unsigned int PopulateCommandList(const Optional<CameraParameters>& editorCamera,
		const Framebuffer& targetFramebuffer);

	void UpdateUniformBuffers(size_t objectDrawCount);

	bool IsDrawCommand(uint64_t orderKey);
	bool ParseControlCommand(uint64_t orderKey);

	void RenderDeferredLighting(const CustomRenderer::RenderParams& params);
	void RenderSkybox(const CustomRenderer::RenderParams& params);
	void RenderPostProcess(const CustomRenderer::RenderParams& params);
	void RenderBloom(const CustomRenderer::RenderParams& params);
	void RenderTonemapping(const CustomRenderer::RenderParams& params);

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

	void Render(const Optional<CameraParameters>& editorCamera, const Framebuffer& targetFramebuffer);

	void DebugRender(DebugVectorRenderer* vectorRenderer, const kokko::RenderDebugSettings& settings);

	virtual void RenderCustom(const CustomRenderer::RenderParams& params) override final;

	// Custom renderer management
	unsigned int AddCustomRenderer(CustomRenderer* customRenderer);
	void RemoveCustomRenderer(unsigned int callbackId);
};
