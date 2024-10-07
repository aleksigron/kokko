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

namespace kokko
{

World::World(AllocatorManager* allocManager,
	Allocator* allocator,
	Allocator* debugNameAllocator,
	render::Device* renderDevice,
	render::CommandEncoder* commandEncoder,
	AssetLoader* assetLoader,
	const ResourceManagers& resourceManagers,
	const RenderDebugSettings* renderDebug) :
	allocator(allocator),
	commandEncoder(commandEncoder),
	levelSerializer(allocator, renderDevice),
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
		resourceManagers.shaderManager, resourceManagers.modelManager, resourceManagers.textureManager);

	meshComponentSystem.CreateScope(allocManager, "MeshComponentSystem", allocator);
	meshComponentSystem.New(meshComponentSystem.allocator, resourceManagers.modelManager);

	renderer.CreateScope(allocManager, "Renderer", allocator);
	renderer.New(renderer.allocator, renderDevice, commandEncoder, meshComponentSystem.instance, scene.instance,
		cameraSystem.instance, lightManager.instance, environmentSystem.instance, resourceManagers, renderDebug);

	scriptSystem.CreateScope(allocManager, "ScriptSystem", allocator);
	scriptSystem.New(scriptSystem.allocator);

	terrainSystem.CreateScope(allocManager, "TerrainSystem", allocator);
	terrainSystem.New(terrainSystem.allocator, assetLoader, renderDevice, resourceManagers.shaderManager,
		resourceManagers.textureManager);

	particleSystem.CreateScope(allocManager, "ParticleEffects", allocator);
	particleSystem.New(
		particleSystem.allocator, renderDevice, resourceManagers.shaderManager, resourceManagers.modelManager);

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
    
#ifdef KOKKO_USE_METAL
    return;
#endif

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

void World::Render(Window* window, const Optional<CameraParameters>& editorCamera, const render::Framebuffer& framebuffer)
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

	environmentSystem.instance->Upload(commandEncoder);

	renderer.instance->Render(window, editorCamera, framebuffer);
}

void World::DebugRender(DebugVectorRenderer* vectorRenderer)
{
	renderer.instance->DebugRender(vectorRenderer);
}

} // namespace kokko
