#pragma once

class Allocator;
class AllocatorManager;
class Window;
class EditorUI;
class Time;
class RenderDevice;
class EntityManager;
class World;
class Renderer;
class MeshManager;
class ShaderManager;
class MaterialManager;
class TextureManager;
class LightManager;
class CameraSystem;
class TerrainManager;
class ParticleSystem;
class EnvironmentManager;
class ScriptSystem;

class Debug;

class Engine
{
private:
	template <typename Type>
	struct InstanceAllocatorPair
	{
		void CreateScope(AllocatorManager* manager, const char* name, Allocator* alloc);

		template <typename... Args>
		void New(Args... args) { instance = allocator->MakeNew<Type>(args...); }

		void Delete()
		{
			allocator->MakeDelete(instance);
			instance = nullptr;
		}

		Type* instance;
		Allocator* allocator;
	};

	AllocatorManager* allocatorManager;

	Allocator* systemAllocator;
	Allocator* debugNameAllocator;

	InstanceAllocatorPair<Window> mainWindow;
	InstanceAllocatorPair<EditorUI> editorUI;
	Time* time;
	RenderDevice* renderDevice;
	InstanceAllocatorPair<Debug> debug;
	InstanceAllocatorPair<EntityManager> entityManager;
	InstanceAllocatorPair<MeshManager> meshManager;
	InstanceAllocatorPair<TextureManager> textureManager;
	InstanceAllocatorPair<ShaderManager> shaderManager;
	InstanceAllocatorPair<MaterialManager> materialManager;
	InstanceAllocatorPair<LightManager> lightManager;
	InstanceAllocatorPair<CameraSystem> cameraSystem;
	InstanceAllocatorPair<TerrainManager> terrainManager;
	InstanceAllocatorPair<ParticleSystem> particleSystem;
	InstanceAllocatorPair<EnvironmentManager> environmentManager;
	InstanceAllocatorPair<World> world;
	InstanceAllocatorPair<Renderer> renderer;
	InstanceAllocatorPair<ScriptSystem> scriptSystem;


public:
	Engine();
	~Engine();

	bool Initialize();
	void FrameStart();
	void Update();

	void SetAppPointer(void* app);

	AllocatorManager* GetAllocatorManager() { return allocatorManager; }
	Window* GetMainWindow() { return mainWindow.instance; }
	EntityManager* GetEntityManager() { return entityManager.instance; }
	MeshManager* GetMeshManager() { return meshManager.instance; }
	MaterialManager* GetMaterialManager() { return materialManager.instance; }
	LightManager* GetLightManager() { return lightManager.instance; }
	CameraSystem* GetCameraSystem() { return cameraSystem.instance; }
	EnvironmentManager* GetEnvironmentManager() { return environmentManager.instance; }
	World* GetWorld() { return world.instance; }
	Renderer* GetRenderer() { return renderer.instance; }
	ScriptSystem* GetScriptSystem() { return scriptSystem.instance; }
};
