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
class TerrainSystem;
class ParticleSystem;

class ComponentSerializer;

class World
{
public:
	World(AllocatorManager* allocManager,
		Allocator* allocator,
		Allocator* debugNameAllocator,
		RenderDevice* renderDevice,
		Filesystem* filesystem,
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
	void DebugRender(EngineSettings* engineSettings, DebugVectorRenderer* vectorRenderer);

	const String& GetLoadedLevelFilename() { return loadedLevelDisplayName; }

	EntityManager* GetEntityManager() { return entityManager.instance; }
	Scene* GetScene() { return scene.instance; }
	Renderer* GetRenderer() { return renderer.instance; }
	LightManager* GetLightManager() { return lightManager.instance; }
	CameraSystem* GetCameraSystem() { return cameraSystem.instance; }
	ScriptSystem* GetScriptSystem() { return scriptSystem.instance; }
	TerrainSystem* GetTerrainSystem() { return terrainSystem.instance; }
	ParticleSystem* GetParticleSystem() { return particleSystem.instance; }

	LevelSerializer* GetSerializer() { return &levelSerializer; }

private:
	Allocator* allocator;
	Filesystem* filesystem;

	LevelSerializer levelSerializer;

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
