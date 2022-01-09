#include "Engine/World.hpp"

#include "Core/Core.hpp"

#include "Debug/Debug.hpp"

#include "Engine/EngineSettings.hpp"
#include "Engine/EntityManager.hpp"

#include "Graphics/EnvironmentSystem.hpp"
#include "Graphics/ParticleSystem.hpp"
#include "Graphics/Scene.hpp"
#include "Graphics/TerrainSystem.hpp"

#include "Math/Rectangle.hpp"

#include "Memory/Memory.hpp"

#include "Rendering/CameraParameters.hpp"
#include "Rendering/CameraSystem.hpp"
#include "Rendering/LightManager.hpp"
#include "Rendering/Renderer.hpp"

#include "Scripting/ScriptSystem.hpp"

#include "System/Filesystem.hpp"
#include "System/Window.hpp"

static const char* const UnnamedLevelDisplayName = "Unnamed level";

World::World(AllocatorManager* allocManager,
	Allocator* allocator,
	Allocator* debugNameAllocator,
	RenderDevice* renderDevice,
	Filesystem* filesystem,
	kokko::AssetLoader* assetLoader,
	InputManager* inputManager,
	const ResourceManagers& resourceManagers) :
	allocator(allocator),
	filesystem(filesystem),
	assetLoader(assetLoader),
	levelSerializer(allocator),
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

	environmentSystem.CreateScope(allocManager, "EnvironmentSystem", alloc);
	environmentSystem.New(environmentSystem.allocator, assetLoader, renderDevice,
		resourceManagers.shaderManager, resourceManagers.meshManager, resourceManagers.textureManager);

	renderer.CreateScope(allocManager, "Renderer", alloc);
	renderer.New(renderer.allocator, renderDevice, scene.instance, cameraSystem.instance,
		lightManager.instance, environmentSystem.instance, resourceManagers);

	scriptSystem.CreateScope(allocManager, "ScriptSystem", alloc);
	scriptSystem.New(scriptSystem.allocator, inputManager);

	terrainSystem.CreateScope(allocManager, "TerrainSystem", alloc);
	terrainSystem.New(terrainSystem.allocator, renderDevice,
		resourceManagers.materialManager, resourceManagers.shaderManager);

	particleSystem.CreateScope(allocManager, "ParticleEffects", alloc);
	particleSystem.New(particleSystem.allocator, renderDevice, resourceManagers.shaderManager, resourceManagers.meshManager);
	levelSerializer.Initialize(this, resourceManagers);
}

World::~World()
{
	particleSystem.Delete();
	terrainSystem.Delete();
	scriptSystem.Delete();
	renderer.Delete();
	scene.Delete();
	environmentSystem.Delete();
	cameraSystem.Delete();
	lightManager.Delete();
	entityManager.Delete();
}

void World::Initialize()
{
	renderer.instance->Initialize();
	terrainSystem.instance->Initialize();
	particleSystem.instance->Initialize();
	environmentSystem.instance->Initialize();
}

void World::Deinitialize()
{
	renderer.instance->Deinitialize();
}

bool World::LoadFromFile(const char* path, const char* displayName)
{
	KOKKO_PROFILE_FUNCTION();

	kokko::String sceneConfig(allocator);

	if (filesystem->ReadText(path, sceneConfig))
	{
		levelSerializer.DeserializeFromString(sceneConfig.GetData());

		loadedLevelDisplayName.Assign(displayName);
		loadedLevelFilePath.Assign(path);

		return true;
	}

	return false;
}

bool World::WriteToFile(const char* path, const char* displayName)
{
	if (levelSerializer.SerializeToFile(path))
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
	environmentSystem.instance->RemoveAll();
	particleSystem.instance->RemoveAll();
	terrainSystem.instance->RemoveAll();
	// TODO: scriptSystem
	cameraSystem.instance->RemoveAll();
	lightManager.instance->RemoveAll();
	renderer.instance->RemoveAll();
	scene.instance->Clear();
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
	TransformUpdateReceiver* transformUpdateReceivers[] =
	{
		lightManager.instance,
		renderer.instance,
		particleSystem.instance
	};

	unsigned int receiverCount = sizeof(transformUpdateReceivers) / sizeof(transformUpdateReceivers[0]);
	scene.instance->NotifyUpdatedTransforms(receiverCount, transformUpdateReceivers);

	terrainSystem.instance->RegisterCustomRenderer(renderer.instance);
	particleSystem.instance->RegisterCustomRenderer(renderer.instance);

	renderer.instance->Render(editorCamera, framebuffer);
}

void World::DebugRender(EngineSettings* engineSettings, DebugVectorRenderer* vectorRenderer)
{
	if (engineSettings->drawMeshBounds)
		renderer.instance->DebugRender(vectorRenderer);
}
