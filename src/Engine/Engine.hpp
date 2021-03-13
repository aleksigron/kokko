#pragma once

class Allocator;
class AllocatorManager;
class Window;
class Time;
class RenderDevice;
class EntityManager;
class Renderer;
class MeshManager;
class ShaderManager;
class MaterialManager;
class TextureManager;
class SceneManager;
class LightManager;
class TerrainManager;
class ParticleSystem;
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

	InstanceAllocatorPair<Window> mainWindow;
	Time* time;
	RenderDevice* renderDevice;
	InstanceAllocatorPair<Debug> debug;
	InstanceAllocatorPair<EntityManager> entityManager;
	InstanceAllocatorPair<MeshManager> meshManager;
	InstanceAllocatorPair<TextureManager> textureManager;
	InstanceAllocatorPair<ShaderManager> shaderManager;
	InstanceAllocatorPair<MaterialManager> materialManager;
	InstanceAllocatorPair<LightManager> lightManager;
	InstanceAllocatorPair<SceneManager> sceneManager;
	InstanceAllocatorPair<TerrainManager> terrainManager;
	InstanceAllocatorPair<ParticleSystem> particleSystem;
	InstanceAllocatorPair<Renderer> renderer;
	InstanceAllocatorPair<ScriptSystem> scriptSystem;


public:
	Engine();
	~Engine();

	bool Initialize();
	void Update();

	void SetAppPointer(void* app);

	AllocatorManager* GetAllocatorManager() { return allocatorManager; }
	Window* GetMainWindow() { return mainWindow.instance; }
	EntityManager* GetEntityManager() { return entityManager.instance; }
	LightManager* GetLightManager() { return lightManager.instance; }
	Renderer* GetRenderer() { return renderer.instance; }
	MaterialManager* GetMaterialManager() { return materialManager.instance; }
	MeshManager* GetMeshManager() { return meshManager.instance; }
	SceneManager* GetSceneManager() { return sceneManager.instance; }
	ScriptSystem* GetScriptSystem() { return scriptSystem.instance; }
};
