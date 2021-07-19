#include "Resources/LevelWriter.hpp"

#include <fstream>

#include "yaml-cpp/yaml.h"

#include "Engine/World.hpp"

#include "Engine/EntityManager.hpp"

#include "Graphics/EnvironmentManager.hpp"
#include "Graphics/ParticleSystem.hpp"
#include "Graphics/Scene.hpp"
#include "Graphics/TerrainSystem.hpp"

#include "Rendering/CameraSystem.hpp"
#include "Rendering/LightManager.hpp"
#include "Rendering/Renderer.hpp"

#include "Resources/MaterialManager.hpp"
#include "Resources/MeshManager.hpp"
#include "Resources/YamlCustomTypes.hpp"

static const char* const ComponentTypeKey = "component_type";

LevelWriter::LevelWriter(World* world, const ResourceManagers& resManagers) :
	world(world),
	resourceManagers(resManagers)
{
}

bool LevelWriter::WriteToFile(const char* filePath)
{
	EntityManager* entityManager = world->GetEntityManager();
	Scene* scene = world->GetScene();

	std::ofstream outStream(filePath);

	if (outStream.is_open() == false)
		return false;

	YAML::Emitter out(outStream);

	out << YAML::BeginMap;

	int environmentId = scene->GetEnvironmentId();
	if (environmentId >= 0)
	{
		const char* sourcePath = resourceManagers.environmentManager->GetEnvironmentSourcePath(environmentId);
		out << YAML::Key << "environment" << YAML::Value << sourcePath;
	}

	out << YAML::Key << "objects" << YAML::Value << YAML::BeginSeq;
	for (Entity entity : *entityManager)
	{
		SceneObjectId sceneObj = scene->Lookup(entity);
		if (sceneObj == SceneObjectId::Null || scene->GetParent(sceneObj) == SceneObjectId::Null)
			WriteEntity(out, entity, sceneObj);
	}
	out << YAML::EndSeq; // objects

	out << YAML::EndMap;

	return true;
}

void LevelWriter::WriteEntity(YAML::Emitter& out, Entity entity, SceneObjectId sceneObj)
{
	EntityManager* entityManager = world->GetEntityManager();
	Scene* scene = world->GetScene();

	out << YAML::BeginMap;

	const char* name = entityManager->GetDebugName(entity);
	if (name != nullptr)
		out << YAML::Key << "entity_name" << YAML::Value << name;

	out << YAML::Key << "components" << YAML::Value << YAML::BeginSeq;

	WriteTransformComponent(out, entity, sceneObj);
	WriteRenderComponent(out, entity);
	WriteLightComponent(out, entity);
	WriteCameraComponent(out, entity);
	WriteTerrainComponent(out, entity);
	WriteParticleComponent(out, entity);

	out << YAML::EndSeq; // components

	SceneObjectId firstChild = scene->GetFirstChild(sceneObj);
	if (firstChild != SceneObjectId::Null)
	{
		out << YAML::Key << "children" << YAML::Value << YAML::BeginSeq;

		SceneObjectId child = firstChild;
		while (child != SceneObjectId::Null)
		{
			Entity childEntity = scene->GetEntity(child);

			WriteEntity(out, childEntity, child);

			child = scene->GetNextSibling(child);
		}

		out << YAML::EndSeq;
	}

	out << YAML::EndMap;
}

void LevelWriter::WriteTransformComponent(YAML::Emitter& out, Entity entity, SceneObjectId sceneObj)
{
	Scene* scene = world->GetScene();

	if (sceneObj != SceneObjectId::Null)
	{
		const SceneEditTransform& transform = scene->GetEditTransform(sceneObj);

		out << YAML::BeginMap;
		out << YAML::Key << ComponentTypeKey << YAML::Value << "transform";
		out << YAML::Key << "position" << YAML::Value << transform.translation;
		out << YAML::Key << "rotation" << YAML::Value << transform.rotation;
		out << YAML::Key << "scale" << YAML::Value << transform.scale;
		out << YAML::EndMap;
	}
}

void LevelWriter::WriteRenderComponent(YAML::Emitter& out, Entity entity)
{
	Renderer* renderer = world->GetRenderer();

	RenderObjectId renderObj = renderer->Lookup(entity);
	if (renderObj != RenderObjectId::Null)
	{
		MeshId meshId = renderer->GetMeshId(renderObj);
		const char* meshPath = resourceManagers.meshManager->GetPath(meshId);

		MaterialId materialId = renderer->GetOrderData(renderObj).material;
		const MaterialData& material = resourceManagers.materialManager->GetMaterialData(materialId);
		const char* materialPath = material.materialPath;

		// We can't reference resources that have been created at runtime
		if (meshPath != nullptr && materialPath != nullptr)
		{
			out << YAML::BeginMap;
			out << YAML::Key << ComponentTypeKey << YAML::Value << "render";
			out << YAML::Key << "mesh" << YAML::Value << meshPath;
			out << YAML::Key << "material" << YAML::Value << materialPath;
			out << YAML::EndMap;
		}
	}
}

void LevelWriter::WriteLightComponent(YAML::Emitter& out, Entity entity)
{
	LightManager* lightManager = world->GetLightManager();

	LightId lightId = lightManager->Lookup(entity);
	if (lightId != LightId::Null)
	{
		out << YAML::BeginMap;
		out << YAML::Key << ComponentTypeKey << YAML::Value << "light";

		LightType lightType = lightManager->GetLightType(lightId);
		out << YAML::Key << "type" << YAML::Value << LightManager::GetLightTypeName(lightType);

		Vec3f lightColor = lightManager->GetColor(lightId);
		out << YAML::Key << "color" << YAML::Value << lightColor;

		if (lightType != LightType::Directional)
		{
			float radius = lightManager->GetRadius(lightId);
			out << YAML::Key << "radius" << YAML::Value << radius;
		}

		if (lightType == LightType::Spot)
		{
			float spotAngle = lightManager->GetSpotAngle(lightId);
			out << YAML::Key << "spot_angle" << YAML::Value << spotAngle;
		}

		bool shadowCasting = lightManager->GetShadowCasting(lightId);
		out << YAML::Key << "cast_shadow" << YAML::Value << shadowCasting;

		out << YAML::EndMap;
	}
}

void LevelWriter::WriteCameraComponent(YAML::Emitter& out, Entity entity)
{
	CameraSystem* cameraSystem = world->GetCameraSystem();

	CameraId cameraId = cameraSystem->Lookup(entity);
	if (cameraId != CameraId::Null())
	{
		out << YAML::BeginMap;
		out << YAML::Key << ComponentTypeKey << YAML::Value << "camera";

		ProjectionParameters params = cameraSystem->GetData(cameraId);

		out << YAML::Key << "projection_type" << YAML::Value << CameraSystem::GetProjectionTypeName(params.projection);

		if (params.projection == ProjectionType::Perspective)
		{
			out << YAML::Key << "field_of_view" << YAML::Value << params.perspectiveFieldOfView;
			out << YAML::Key << "near" << YAML::Value << params.perspectiveNear;
			out << YAML::Key << "far" << YAML::Value << params.perspectiveFar;
		}
		else
		{
			out << YAML::Key << "orthographic_height" << YAML::Value << params.orthographicHeight;
			out << YAML::Key << "near" << YAML::Value << params.orthographicNear;
			out << YAML::Key << "far" << YAML::Value << params.orthographicFar;
		}

		out << YAML::EndMap;
	}
}

void LevelWriter::WriteTerrainComponent(YAML::Emitter& out, Entity entity)
{
	TerrainSystem* terrainSystem = world->GetTerrainSystem();

	TerrainId terrainId = terrainSystem->Lookup(entity);
	if (terrainId != TerrainId::Null())
	{
		out << YAML::BeginMap;
		out << YAML::Key << ComponentTypeKey << YAML::Value << "terrain";

		const TerrainInstance& terrain = terrainSystem->GetData(terrainId);

		out << YAML::Key << "terrain_size" << YAML::Value << terrain.terrainSize;
		out << YAML::Key << "terrain_resolution" << YAML::Value << terrain.terrainResolution;
		out << YAML::Key << "texture_scale" << YAML::Value << terrain.textureScale;
		out << YAML::Key << "min_height" << YAML::Value << terrain.minHeight;
		out << YAML::Key << "max_height" << YAML::Value << terrain.maxHeight;

		out << YAML::EndMap;
	}
}

void LevelWriter::WriteParticleComponent(YAML::Emitter& out, Entity entity)
{
	ParticleSystem* particleSystem = world->GetParticleSystem();

	ParticleEmitterId emitterId = particleSystem->Lookup(entity);
	if (emitterId != ParticleEmitterId::Null)
	{
		out << YAML::BeginMap;
		out << YAML::Key << ComponentTypeKey << YAML::Value << "particle";

		float emitRate = particleSystem->GetEmitRate(emitterId);
		out << YAML::Key << "emit_rate" << YAML::Value << emitRate;

		out << YAML::EndMap;
	}
}
