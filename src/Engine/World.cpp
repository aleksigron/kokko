#include "Engine/World.hpp"

#include "Debug/Debug.hpp"

#include "Entity/EntityManager.hpp"

#include "Graphics/ParticleSystem.hpp"
#include "Graphics/Scene.hpp"
#include "Graphics/TerrainManager.hpp"

#include "Math/Rectangle.hpp"

#include "Memory/Memory.hpp"

#include "Rendering/CameraParameters.hpp"
#include "Rendering/CameraSystem.hpp"
#include "Rendering/LightManager.hpp"
#include "Rendering/Renderer.hpp"

#include "Resources/ResourceManagers.hpp"

#include "Scripting/ScriptSystem.hpp"

#include "System/Window.hpp"

World::World(AllocatorManager* allocManager,
	Allocator* allocator,
	Allocator* debugNameAllocator,
	RenderDevice* renderDevice,
	InputManager* inputManager,
	const ResourceManagers& resourceManagers)
{
	Allocator* alloc = Memory::GetDefaultAllocator();

	entityManager.CreateScope(allocManager, "EntityManager", alloc);
	entityManager.New(entityManager.allocator, debugNameAllocator);

	lightManager.CreateScope(allocManager, "LightManager", alloc);
	lightManager.New(lightManager.allocator);

	cameraSystem.CreateScope(allocManager, "CameraSystem", alloc);
	cameraSystem.New(cameraSystem.allocator);

	scene.CreateScope(allocManager, "Scene", alloc);
	scene.New(scene.allocator, this, resourceManagers);

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

void World::Initialize(Window* window)
{
	renderer.instance->Initialize(window);
	terrainManager.instance->Initialize();
	particleSystem.instance->Initialize();
}

void World::Deinitialize()
{
	renderer.instance->Deinitialize();
}

void World::Update()
{
	scriptSystem.instance->UpdateScripts(this);
}

void World::Render(const Optional<CameraParameters>& editorCamera, const ViewRectangle& viewport)
{
	// Propagate transform updates from Scene to other systems that require it
	TransformUpdateReceiver* transformUpdateReceivers[] = { lightManager.instance, renderer.instance };
	unsigned int receiverCount = sizeof(transformUpdateReceivers) / sizeof(transformUpdateReceivers[0]);
	scene.instance->NotifyUpdatedTransforms(receiverCount, transformUpdateReceivers);

	renderer.instance->SetFullscreenViewportRectangle(viewport);

	terrainManager.instance->RegisterCustomRenderer(renderer.instance);
	particleSystem.instance->RegisterCustomRenderer(renderer.instance);

	renderer.instance->Render(editorCamera);
}

void World::DebugRender(DebugVectorRenderer* vectorRenderer)
{
	renderer.instance->DebugRender(vectorRenderer);
}