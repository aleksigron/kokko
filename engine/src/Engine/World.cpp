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
#include "Rendering/MeshComponentSystem.hpp"
#include "Rendering/Renderer.hpp"

#include "Resources/AssetLoader.hpp"

#include "Scripting/ScriptSystem.hpp"

#include "System/Window.hpp"

World::World(AllocatorManager* allocManager,
	Allocator* allocator,
	Allocator* debugNameAllocator,
	RenderDevice* renderDevice,
	kokko::AssetLoader* assetLoader,
	InputManager* inputManager,
	const kokko::ResourceManagers& resourceManagers) :
	allocator(allocator),
	levelSerializer(allocator),
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

	meshComponentSystem.CreateScope(allocManager, "MeshComponentSystem", alloc);
	meshComponentSystem.New(meshComponentSystem.allocator, resourceManagers.meshManager);

	renderer.CreateScope(allocManager, "Renderer", alloc);
	renderer.New(renderer.allocator, renderDevice, meshComponentSystem.instance, scene.instance,
		cameraSystem.instance, lightManager.instance, environmentSystem.instance, resourceManagers);

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
	meshComponentSystem.Delete();
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

void World::ClearAllEntities()
{
	environmentSystem.instance->RemoveAll();
	particleSystem.instance->RemoveAll();
	terrainSystem.instance->RemoveAll();
	// TODO: scriptSystem
	cameraSystem.instance->RemoveAll();
	lightManager.instance->RemoveAll();
	meshComponentSystem.instance->RemoveAll();
	scene.instance->Clear();
	entityManager.instance->ClearAll();
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
		meshComponentSystem.instance,
		particleSystem.instance
	};

	unsigned int receiverCount = sizeof(transformUpdateReceivers) / sizeof(transformUpdateReceivers[0]);
	scene.instance->NotifyUpdatedTransforms(receiverCount, transformUpdateReceivers);

	terrainSystem.instance->RegisterCustomRenderer(renderer.instance);
	particleSystem.instance->RegisterCustomRenderer(renderer.instance);

	renderer.instance->Render(editorCamera, framebuffer);
}

void World::DebugRender(DebugVectorRenderer* vectorRenderer, const kokko::RenderDebugSettings& renderDebug)
{
	renderer.instance->DebugRender(vectorRenderer, renderDebug);
}
