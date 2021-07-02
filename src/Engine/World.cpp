#include "Engine/World.hpp"

#include "Core/Core.hpp"

#include "Debug/Debug.hpp"

#include "Engine/EntityManager.hpp"

#include "Graphics/ParticleSystem.hpp"
#include "Graphics/Scene.hpp"
#include "Graphics/TerrainManager.hpp"

#include "Math/Rectangle.hpp"

#include "Memory/Memory.hpp"

#include "Rendering/CameraParameters.hpp"
#include "Rendering/CameraSystem.hpp"
#include "Rendering/LightManager.hpp"
#include "Rendering/Renderer.hpp"

#include "Resources/LevelLoader.hpp"
#include "Resources/LevelWriter.hpp"
#include "Resources/ResourceManagers.hpp"

#include "Scripting/ScriptSystem.hpp"

#include "System/File.hpp"
#include "System/Window.hpp"

static const char* const UnnamedLevelDisplayName = "Unnamed level";

World::World(AllocatorManager* allocManager,
	Allocator* allocator,
	Allocator* debugNameAllocator,
	RenderDevice* renderDevice,
	InputManager* inputManager,
	const ResourceManagers& resourceManagers) :
	allocator(allocator),
	loadedLevelDisplayName(allocator, UnnamedLevelDisplayName),
	loadedLevelFilePath(allocator),
	resourceManagers(resourceManagers)
{
	Allocator* alloc = Memory::GetDefaultAllocator();

	entityManager.CreateScope(allocManager, "EntityManager", alloc);
	entityManager.New(entityManager.allocator, debugNameAllocator);

	lightManager.CreateScope(allocManager, "LightManager", alloc);
	lightManager.New(lightManager.allocator);

	cameraSystem.CreateScope(allocManager, "CameraSystem", alloc);
	cameraSystem.New(cameraSystem.allocator);

	scene.CreateScope(allocManager, "Scene", alloc);
	scene.New(scene.allocator);

	renderer.CreateScope(allocManager, "Renderer", alloc);
	renderer.New(renderer.allocator, renderDevice, scene.instance, cameraSystem.instance,
		lightManager.instance, resourceManagers);

	scriptSystem.CreateScope(allocManager, "ScriptSystem", alloc);
	scriptSystem.New(scriptSystem.allocator, inputManager);

	terrainManager.CreateScope(allocManager, "TerrainManager", alloc);
	terrainManager.New(terrainManager.allocator, renderDevice, resourceManagers.meshManager,
		resourceManagers.materialManager, resourceManagers.shaderManager);

	particleSystem.CreateScope(allocManager, "ParticleEffects", alloc);
	particleSystem.New(particleSystem.allocator, renderDevice, resourceManagers.shaderManager, resourceManagers.meshManager);
}

World::~World()
{
	particleSystem.Delete();
	terrainManager.Delete();
	scriptSystem.Delete();
	renderer.Delete();
	scene.Delete();
	cameraSystem.Delete();
	lightManager.Delete();
	entityManager.Delete();
}

void World::Initialize()
{
	renderer.instance->Initialize();
	terrainManager.instance->Initialize();
	particleSystem.instance->Initialize();
}

void World::Deinitialize()
{
	renderer.instance->Deinitialize();
}

bool World::LoadFromFile(const char* path, const char* displayName)
{
	KOKKO_PROFILE_FUNCTION();

	Array<char> sceneConfig(allocator);

	if (File::ReadText(path, sceneConfig))
	{
		LevelLoader loader(this, resourceManagers);
		loader.Load(sceneConfig.GetData());

		loadedLevelDisplayName.Assign(displayName);
		loadedLevelFilePath.Assign(path);

		return true;
	}

	return false;
}

bool World::WriteToFile(const char* path, const char* displayName)
{
	LevelWriter writer(this, resourceManagers);
	
	if (writer.WriteToFile(path))
	{
		loadedLevelDisplayName.Assign(displayName);
		loadedLevelFilePath.Assign(path);

		return true;
	}
	else
		return false;
}

void World::ClearAllEntities()
{
	cameraSystem.instance->RemoveAll();
	lightManager.instance->RemoveAll();
	renderer.instance->RemoveAll();
	scene.instance->Clear();
	// TODO: scriptSystem
	entityManager.instance->ClearAll();

	loadedLevelDisplayName.Assign(UnnamedLevelDisplayName);
	loadedLevelFilePath.Clear();
}

void World::Update()
{
	scriptSystem.instance->UpdateScripts(this);
}

void World::Render(const Optional<CameraParameters>& editorCamera, const Framebuffer& framebuffer)
{
	// Propagate transform updates from Scene to other systems that require it
	TransformUpdateReceiver* transformUpdateReceivers[] = { lightManager.instance, renderer.instance };
	unsigned int receiverCount = sizeof(transformUpdateReceivers) / sizeof(transformUpdateReceivers[0]);
	scene.instance->NotifyUpdatedTransforms(receiverCount, transformUpdateReceivers);

	terrainManager.instance->RegisterCustomRenderer(renderer.instance);
	particleSystem.instance->RegisterCustomRenderer(renderer.instance);

	renderer.instance->Render(editorCamera, framebuffer);
}

void World::DebugRender(DebugVectorRenderer* vectorRenderer)
{
	renderer.instance->DebugRender(vectorRenderer);
}
