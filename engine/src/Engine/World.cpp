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

#include "Rendering/CameraParameters.hpp"
#include "Rendering/CameraSystem.hpp"
#include "Rendering/LightManager.hpp"
#include "Rendering/MeshComponentSystem.hpp"
#include "Rendering/Renderer.hpp"

#include "Resources/AssetLoader.hpp"

#include "Scripting/ScriptSystem.hpp"

#include "Platform/Window.hpp"

World::World(AllocatorManager* allocManager,
	Allocator* allocator,
	Allocator* debugNameAllocator,
	RenderDevice* renderDevice,
	kokko::AssetLoader* assetLoader,
	const kokko::ResourceManagers& resourceManagers) :
	allocator(allocator),
	levelSerializer(allocator),
	resourceManagers(resourceManagers)
{
	entityManager.CreateScope(allocManager, "EntityManager", allocator);
	entityManager.New(entityManager.allocator, debugNameAllocator);

	lightManager.CreateScope(allocManager, "LightManager", allocator);
	lightManager.New(lightManager.allocator);

	cameraSystem.CreateScope(allocManager, "CameraSystem", allocator);
	cameraSystem.New(cameraSystem.allocator);

	scene.CreateScope(allocManager, "Scene", allocator);
	scene.New(scene.allocator);

	environmentSystem.CreateScope(allocManager, "EnvironmentSystem", allocator);
	environmentSystem.New(environmentSystem.allocator, assetLoader, renderDevice,
		resourceManagers.shaderManager, resourceManagers.meshManager, resourceManagers.textureManager);

	meshComponentSystem.CreateScope(allocManager, "MeshComponentSystem", allocator);
	meshComponentSystem.New(meshComponentSystem.allocator, resourceManagers.meshManager);

	renderer.CreateScope(allocManager, "Renderer", allocator);
	renderer.New(renderer.allocator, renderDevice, meshComponentSystem.instance, scene.instance,
		cameraSystem.instance, lightManager.instance, environmentSystem.instance, resourceManagers);

	scriptSystem.CreateScope(allocManager, "ScriptSystem", allocator);
	scriptSystem.New(scriptSystem.allocator);

	terrainSystem.CreateScope(allocManager, "TerrainSystem", allocator);
	terrainSystem.New(terrainSystem.allocator, renderDevice,
		resourceManagers.materialManager, resourceManagers.shaderManager);

	particleSystem.CreateScope(allocManager, "ParticleEffects", allocator);
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
	environmentSystem.Delete();
	scene.Delete();
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

	renderer.instance->AddGraphicsFeature(particleSystem.instance);
	renderer.instance->AddGraphicsFeature(terrainSystem.instance);
}

void World::Deinitialize()
{
	renderer.instance->RemoveGraphicsFeature(terrainSystem.instance);
	renderer.instance->RemoveGraphicsFeature(particleSystem.instance);

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

void World::Update(InputManager* inputManager)
{
	scriptSystem.instance->UpdateScripts(this, inputManager);
}

void World::Render(kokko::Window* window, const Optional<CameraParameters>& editorCamera, const Framebuffer& framebuffer)
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

	renderer.instance->Render(window, editorCamera, framebuffer);
}

void World::DebugRender(DebugVectorRenderer* vectorRenderer, const kokko::RenderDebugSettings& renderDebug)
{
	renderer.instance->DebugRender(vectorRenderer, renderDebug);
}
