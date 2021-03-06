#pragma once

#include "Core/Optional.hpp"
#include "Core/String.hpp"

#include "Engine/InstanceAllocatorPair.hpp"

#include "Resources/ResourceManagers.hpp"

class Allocator;
class AllocatorManager;
class RenderDevice;
class InputManager;
class DebugVectorRenderer;
class Window;
class Framebuffer;

struct CameraParameters;
struct ResourceManagers;

class EntityManager;
class Scene;
class Renderer;
class LightManager;
class CameraSystem;
class ScriptSystem;
class TerrainSystem;
class ParticleSystem;

class World
{
public:
	World(AllocatorManager* allocManager,
		Allocator* allocator,
		Allocator* debugNameAllocator,
		RenderDevice* renderDevice,
		InputManager* inputManager,
		const ResourceManagers& resourceManagers);
	~World();

	void Initialize();
	void Deinitialize();

	bool LoadFromFile(const char* path, const char* displayName);
	bool WriteToFile(const char* path, const char* displayName);

	void ClearAllEntities();

	void Update();
	void Render(const Optional<CameraParameters>& editorCamera, const Framebuffer& framebuffer);
	void DebugRender(DebugVectorRenderer* vectorRenderer);

	const String& GetLoadedLevelFilename() { return loadedLevelDisplayName; }

	EntityManager* GetEntityManager() { return entityManager.instance; }
	Scene* GetScene() { return scene.instance; }
	Renderer* GetRenderer() { return renderer.instance; }
	LightManager* GetLightManager() { return lightManager.instance; }
	CameraSystem* GetCameraSystem() { return cameraSystem.instance; }
	TerrainSystem* GetTerrainSystem() { return terrainSystem.instance; }
	ScriptSystem* GetScriptSystem() { return scriptSystem.instance; }

private:
	Allocator* allocator;
	String loadedLevelDisplayName;
	String loadedLevelFilePath;

	InstanceAllocatorPair<EntityManager> entityManager;
	InstanceAllocatorPair<LightManager> lightManager;
	InstanceAllocatorPair<CameraSystem> cameraSystem;
	InstanceAllocatorPair<Scene> scene;
	InstanceAllocatorPair<Renderer> renderer;
	InstanceAllocatorPair<ScriptSystem> scriptSystem;
	InstanceAllocatorPair<TerrainSystem> terrainSystem;
	InstanceAllocatorPair<ParticleSystem> particleSystem;
	ResourceManagers resourceManagers;
};
