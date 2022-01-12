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
struct EngineSettings;
struct ResourceManagers;

class EntityManager;
class Scene;
class Renderer;
class LightManager;
class CameraSystem;
class ScriptSystem;
class ParticleSystem;

class ComponentSerializer;

namespace kokko
{
class AssetLoader;
class TerrainSystem;
class EnvironmentSystem;
}

class World
{
public:
	World(AllocatorManager* allocManager,
		Allocator* allocator,
		Allocator* debugNameAllocator,
		RenderDevice* renderDevice,
		kokko::AssetLoader* assetLoader,
		InputManager* inputManager,
		const ResourceManagers& resourceManagers);
	~World();

	void Initialize();
	void Deinitialize();

	void ClearAllEntities();

	void Update();
	void Render(const Optional<CameraParameters>& editorCamera, const Framebuffer& framebuffer);
	void DebugRender(EngineSettings* engineSettings, DebugVectorRenderer* vectorRenderer);

	EntityManager* GetEntityManager() { return entityManager.instance; }
	Scene* GetScene() { return scene.instance; }
	Renderer* GetRenderer() { return renderer.instance; }
	LightManager* GetLightManager() { return lightManager.instance; }
	CameraSystem* GetCameraSystem() { return cameraSystem.instance; }
	kokko::EnvironmentSystem* GetEnvironmentSystem() { return environmentSystem.instance; }
	ScriptSystem* GetScriptSystem() { return scriptSystem.instance; }
	kokko::TerrainSystem* GetTerrainSystem() { return terrainSystem.instance; }
	ParticleSystem* GetParticleSystem() { return particleSystem.instance; }

	LevelSerializer* GetSerializer() { return &levelSerializer; }

private:
	Allocator* allocator;

	LevelSerializer levelSerializer;

	InstanceAllocatorPair<EntityManager> entityManager;
	InstanceAllocatorPair<LightManager> lightManager;
	InstanceAllocatorPair<CameraSystem> cameraSystem;
	InstanceAllocatorPair<kokko::EnvironmentSystem> environmentSystem;
	InstanceAllocatorPair<Scene> scene;
	InstanceAllocatorPair<Renderer> renderer;
	InstanceAllocatorPair<ScriptSystem> scriptSystem;
	InstanceAllocatorPair<kokko::TerrainSystem> terrainSystem;
	InstanceAllocatorPair<ParticleSystem> particleSystem;

	ResourceManagers resourceManagers;
};
