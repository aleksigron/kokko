#pragma once

#include "Core/Optional.hpp"
#include "Core/String.hpp"

#include "Engine/InstanceAllocatorPair.hpp"

#include "Resources/LevelSerializer.hpp"
#include "Resources/ResourceManagers.hpp"

class Allocator;
class AllocatorManager;
class InputManager;

struct CameraParameters;

class EntityManager;
class Scene;
class ScriptSystem;

class ComponentSerializer;

namespace kokko
{
class AssetLoader;
class CameraSystem;
class DebugVectorRenderer;
class EnvironmentSystem;
class Filesystem;
class LightManager;
class MeshComponentSystem;
class ParticleSystem;
class Renderer;
class RenderDebugSettings;
class TerrainSystem;
class Window;

namespace render
{
class CommandEncoder;
class Device;
class Framebuffer;
}

class World
{
public:
	World(AllocatorManager* allocManager,
		Allocator* allocator,
		Allocator* debugNameAllocator,
		render::Device* renderDevice,
		render::CommandEncoder* commandEncoder,
		AssetLoader* assetLoader,
		const ResourceManagers& resourceManagers,
		const RenderDebugSettings* renderDebug);
	~World();

	void Initialize();
	void Deinitialize();

	void ClearAllEntities();

	void Update(InputManager* inputManager);
	void Render(Window* window, const Optional<CameraParameters>& editorCamera,
		const render::Framebuffer& framebuffer);
	void DebugRender(DebugVectorRenderer* vectorRenderer);

	EntityManager* GetEntityManager() { return entityManager.instance; }
	Scene* GetScene() { return scene.instance; }
	Renderer* GetRenderer() { return renderer.instance; }
	MeshComponentSystem* GetMeshComponentSystem() { return meshComponentSystem.instance; }
	LightManager* GetLightManager() { return lightManager.instance; }
	CameraSystem* GetCameraSystem() { return cameraSystem.instance; }
	EnvironmentSystem* GetEnvironmentSystem() { return environmentSystem.instance; }
	ScriptSystem* GetScriptSystem() { return scriptSystem.instance; }
	TerrainSystem* GetTerrainSystem() { return terrainSystem.instance; }
	ParticleSystem* GetParticleSystem() { return particleSystem.instance; }

	LevelSerializer* GetSerializer() { return &levelSerializer; }

private:
	Allocator* allocator;
	render::CommandEncoder* commandEncoder;

	LevelSerializer levelSerializer;

	InstanceAllocatorPair<EntityManager> entityManager;
	InstanceAllocatorPair<LightManager> lightManager;
	InstanceAllocatorPair<CameraSystem> cameraSystem;
	InstanceAllocatorPair<EnvironmentSystem> environmentSystem;
	InstanceAllocatorPair<Scene> scene;
	InstanceAllocatorPair<MeshComponentSystem> meshComponentSystem;
	InstanceAllocatorPair<Renderer> renderer;
	InstanceAllocatorPair<ScriptSystem> scriptSystem;
	InstanceAllocatorPair<TerrainSystem> terrainSystem;
	InstanceAllocatorPair<ParticleSystem> particleSystem;

	ResourceManagers resourceManagers;
};

} // namespace kokko