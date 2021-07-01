#pragma once

#include "Core/Array.hpp"
#include "Core/BitPack.hpp"
#include "Core/FixedArray.hpp"
#include "Core/HashMap.hpp"
#include "Core/Optional.hpp"

#include "Entity/Entity.hpp"

#include "Graphics/TransformUpdateReceiver.hpp"

#include "Math/Mat4x4.hpp"
#include "Math/Rectangle.hpp"
#include "Math/Vec3.hpp"
#include "Math/Vec2.hpp"
#include "Math/Projection.hpp"

#include "Resources/MeshData.hpp"
#include "Resources/MaterialData.hpp"
#include "Resources/ShaderId.hpp"

#include "Rendering/CustomRenderer.hpp"
#include "Rendering/Framebuffer.hpp"
#include "Rendering/Light.hpp"
#include "Rendering/RenderCommandList.hpp"
#include "Rendering/RendererData.hpp"
#include "Rendering/RenderOrder.hpp"

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
class EnvironmentManager;
class PostProcessRenderer;
class RenderTargetContainer;
class Framebuffer;

struct BoundingBox;
struct CameraParameters;
struct RenderViewport;
struct MaterialData;
struct ShaderData;
struct ProjectionParameters;
struct LightingUniformBlock;
struct PostProcessRenderPass;
struct ResourceManagers;

class Renderer : public TransformUpdateReceiver, public CustomRenderer
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

	struct InstanceData
	{
		unsigned int count;
		unsigned int allocated;
		void *buffer;

		Entity* entity;
		MeshId* mesh;
		RenderOrderData* order;
		BoundingBox* bounds;
		Mat4x4f* transform;
	}
	data;

	HashMap<unsigned int, RenderObjectId> entityMap;

	RenderOrderConfiguration renderOrder;

	Scene* scene;
	CameraSystem* cameraSystem;
	LightManager* lightManager;
	ShaderManager* shaderManager;
	MeshManager* meshManager;
	MaterialManager* materialManager;
	TextureManager* textureManager;
	EnvironmentManager* environmentManager;

	bool lockCullingCamera;
	Mat4x4fBijection lockCullingCameraTransform;

	RenderCommandList commandList;
	Array<BitPack> objectVisibility;

	Array<LightId> lightResultArray;

	Array<CustomRenderer*> customRenderers;

	ShaderId skyboxShaderId;
	MeshId skyboxMeshId;
	unsigned int skyboxUniformBufferId;

	void ReallocateRenderObjects(unsigned int required);

	void CreateResolutionDependentFramebuffers(int width, int height);
	void DestroyResolutionDependentFramebuffers();

	void BindMaterialTextures(const MaterialData& material) const;
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
		Scene* scene,
		CameraSystem* cameraSystem,
		LightManager* lightManager,
		const ResourceManagers& resourceManagers);
	~Renderer();

	void Initialize();
	void Deinitialize();

	void SetLockCullingCamera(bool lockEnable);
	const Mat4x4f& GetCullingCameraTransform() const;

	void Render(const Optional<CameraParameters>& editorCamera, const Framebuffer& targetFramebuffer);

	void DebugRender(DebugVectorRenderer* vectorRenderer);

	virtual void NotifyUpdatedTransforms(unsigned int count, const Entity* entities, const Mat4x4f* transforms);

	// Render object management

	RenderObjectId Lookup(Entity e)
	{
		HashMap<unsigned int, RenderObjectId>::KeyValuePair* pair = entityMap.Lookup(e.id);
		return pair != nullptr ? pair->second : RenderObjectId{};
	}

	RenderObjectId AddRenderObject(Entity entity);
	void AddRenderObject(unsigned int count, const Entity* entities, RenderObjectId* renderObjectIdsOut);

	void RemoveRenderObject(RenderObjectId id);

	void RemoveAll();

	// Render object property management

	void SetMeshId(RenderObjectId id, MeshId meshId) { data.mesh[id.i] = meshId; }
	MeshId GetMeshId(RenderObjectId id) const { return data.mesh[id.i]; }

	void SetOrderData(RenderObjectId id, const RenderOrderData& order) { data.order[id.i] = order; }
	const RenderOrderData& GetOrderData(RenderObjectId id) const { return data.order[id.i]; }

	virtual void RenderCustom(const CustomRenderer::RenderParams& params) override final;

	// Custom renderer management
	unsigned int AddCustomRenderer(CustomRenderer* customRenderer);
	void RemoveCustomRenderer(unsigned int callbackId);
};
