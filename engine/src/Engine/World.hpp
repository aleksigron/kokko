#pragma once

#include "Core/Optional.hpp"
#include "Core/String.hpp"

#include "Engine/InstanceAllocatorPair.hpp"

#include "Resources/LevelSerializer.hpp"
#include "Resources/ResourceManagers.hpp"

class Allocator;
class AllocatorManager;
class RenderDevice;
class Filesystem;
class InputManager;
class DebugVectorRenderer;
class Window;
class Framebuffer;

struct CameraParameters;

class EntityManager;
class Scene;
class Renderer;
class LightManager;
class CameraSystem;
class ScriptSystem;

class ComponentSerializer;

namespace kokko
{
class AssetLoader;
class EnvironmentSystem;
class MeshComponentSystem;
class ParticleSystem;
class RenderDebugSettings;
class TerrainSystem;
class Window;
}

class World
{
public:
	World(AllocatorManager* allocManager,
		Allocator* allocator,
		Allocator* debugNameAllocator,
		RenderDevice* renderDevice,
		kokko::AssetLoader* assetLoader,
		const kokko::ResourceManagers& resourceManagers);
	~World();

	void Initialize();
	void Deinitialize();

	void ClearAllEntities();

	void Update(InputManager* inputManager);
	void Render(kokko::Window* window, const Optional<CameraParameters>& editorCamera, const Framebuffer& framebuffer);
	void DebugRender(DebugVectorRenderer* vectorRenderer, const kokko::RenderDebugSettings& renderDebug);

	EntityManager* GetEntityManager() { return entityManager.instance; }
	Scene* GetScene() { return scene.instance; }
	Renderer* GetRenderer() { return renderer.instance; }
	kokko::MeshComponentSystem* GetMeshComponentSystem() { return meshComponentSystem.instance; }
	LightManager* GetLightManager() { return lightManager.instance; }
	CameraSystem* GetCameraSystem() { return cameraSystem.instance; }
	kokko::EnvironmentSystem* GetEnvironmentSystem() { return environmentSystem.instance; }
	ScriptSystem* GetScriptSystem() { return scriptSystem.instance; }
	kokko::TerrainSystem* GetTerrainSystem() { return terrainSystem.instance; }
	kokko::ParticleSystem* GetParticleSystem() { return particleSystem.instance; }

	LevelSerializer* GetSerializer() { return &levelSerializer; }

private:
	Allocator* allocator;

	LevelSerializer levelSerializer;

	InstanceAllocatorPair<EntityManager> entityManager;
	InstanceAllocatorPair<LightManager> lightManager;
	InstanceAllocatorPair<CameraSystem> cameraSystem;
	InstanceAllocatorPair<kokko::EnvironmentSystem> environmentSystem;
	InstanceAllocatorPair<Scene> scene;
	InstanceAllocatorPair<kokko::MeshComponentSystem> meshComponentSystem;
	InstanceAllocatorPair<Renderer> renderer;
	InstanceAllocatorPair<ScriptSystem> scriptSystem;
	InstanceAllocatorPair<kokko::TerrainSystem> terrainSystem;
	InstanceAllocatorPair<kokko::ParticleSystem> particleSystem;

	kokko::ResourceManagers resourceManagers;
};
