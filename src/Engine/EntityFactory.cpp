#include "Engine/EntityFactory.hpp"

#include <cassert>

#include "Engine/World.hpp"

#include "Engine/EntityManager.hpp"

#include "Graphics/Scene.hpp"
#include "Graphics/TerrainManager.hpp"

#include "Rendering/CameraSystem.hpp"
#include "Rendering/LightManager.hpp"
#include "Rendering/Renderer.hpp"

const char* const EntityFactory::ComponentNames[] = {
	"Transform",
	"Render object",
	"Camera",
	"Light",
	"Terrain"
};

Entity EntityFactory::CreateEntity(World* world, ArrayView<EntityComponentType> components)
{
	EntityManager* entityManager = world->GetEntityManager();

	Entity entity = entityManager->Create();

	for (EntityComponentType component : components)
		AddComponent(world, entity, component);

	return entity;
}

void EntityFactory::DestroyEntity(World* world, Entity entity)
{
	EntityManager* entityManager = world->GetEntityManager();

	for (size_t i = 0; i < ComponentTypeCount; ++i)
		RemoveComponentIfExists(world, entity, static_cast<EntityComponentType>(i));

	entityManager->Destroy(entity);
}

void EntityFactory::AddComponent(World* world, Entity entity, EntityComponentType componentType)
{
	switch (componentType)
	{
	case EntityComponentType::Scene:
	{
		Scene* scene = world->GetScene();
		SceneObjectId sceneObj = scene->Lookup(entity);
		if (sceneObj == SceneObjectId::Null)
			scene->AddSceneObject(entity);
		break;
	}
	case EntityComponentType::Render:
	{
		Renderer* renderer = world->GetRenderer();
		RenderObjectId renderObj = renderer->Lookup(entity);
		if (renderObj == RenderObjectId::Null)
			renderer->AddRenderObject(entity);
		break;
	}
	case EntityComponentType::Camera:
	{
		CameraSystem* cameraSystem = world->GetCameraSystem();
		CameraId cameraId = cameraSystem->Lookup(entity);
		if (cameraId == CameraId::Null())
			cameraSystem->AddComponentToEntity(entity);
		break;
	}
	case EntityComponentType::Light:
	{
		LightManager* lightManager = world->GetLightManager();
		LightId lightId = lightManager->Lookup(entity);
		if (lightId == LightId::Null)
			lightManager->AddLight(entity);
		break;
	}
	case EntityComponentType::Terrain:
	{
		TerrainManager* terrainManager = world->GetTerrainManager();
		TerrainId terrainId = terrainManager->Lookup(entity);
		if (terrainId == TerrainId::Null())
		{
			terrainId = terrainManager->AddComponentToEntity(entity);
			terrainManager->InitializeTerrain(terrainId);
		}
		break;
	}
	default:
		break;
	}
}

void EntityFactory::RemoveComponentIfExists(World* world, Entity entity, EntityComponentType componentType)
{
	switch (componentType)
	{
	case EntityComponentType::Scene:
	{
		Scene* scene = world->GetScene();
		SceneObjectId sceneObj = scene->Lookup(entity);
		if (sceneObj != SceneObjectId::Null)
			scene->RemoveSceneObject(sceneObj);
		break;
	}
	case EntityComponentType::Render:
	{
		Renderer* renderer = world->GetRenderer();
		RenderObjectId renderObj = renderer->Lookup(entity);
		if (renderObj != RenderObjectId::Null)
			renderer->RemoveRenderObject(renderObj);
		break;
	}
	case EntityComponentType::Camera:
	{
		CameraSystem* cameraSystem = world->GetCameraSystem();
		CameraId cameraId = cameraSystem->Lookup(entity);
		if (cameraId != CameraId::Null())
			cameraSystem->RemoveComponent(cameraId);
		break;
	}
	case EntityComponentType::Light:
	{
		LightManager* lightManager = world->GetLightManager();
		LightId lightId = lightManager->Lookup(entity);
		if (lightId != LightId::Null)
			lightManager->RemoveLight(lightId);
		break;
	}
	case EntityComponentType::Terrain:
	{
		TerrainManager* terrainManager = world->GetTerrainManager();
		TerrainId terrainId = terrainManager->Lookup(entity);
		if (terrainId != TerrainId::Null())
		{
			terrainManager->DeinitializeTerrain(terrainId);
			terrainManager->RemoveComponent(terrainId);
		}
		break;
	}
	default:
		break;
	}
}

const char* EntityFactory::GetComponentTypeName(size_t typeIndex)
{
	assert(typeIndex < ComponentTypeCount);

	return ComponentNames[typeIndex];
}

const char* EntityFactory::GetComponentTypeName(EntityComponentType component)
{
	return GetComponentTypeName(static_cast<size_t>(component));
}
