#include "Resources/LevelWriter.hpp"

#include <fstream>

#include "yaml-cpp/yaml.h"

#include "Engine/Engine.hpp"

#include "Entity/EntityManager.hpp"

#include "Graphics/EnvironmentManager.hpp"
#include "Graphics/Scene.hpp"

#include "Rendering/CameraSystem.hpp"
#include "Rendering/LightManager.hpp"
#include "Rendering/Renderer.hpp"

#include "Resources/MaterialManager.hpp"
#include "Resources/MeshManager.hpp"
#include "Resources/YamlCustomTypes.hpp"

static const char* const ComponentTypeKey = "component_type";

LevelWriter::LevelWriter(Engine* engine) :
	entityManager(engine->GetEntityManager()),
	world(engine->GetWorld()),
	renderer(engine->GetRenderer()),
	lightManager(engine->GetLightManager()),
	cameraSystem(engine->GetCameraSystem()),
	meshManager(engine->GetMeshManager()),
	materialManager(engine->GetMaterialManager()),
	environmentManager(engine->GetEnvironmentManager())
{
}

bool LevelWriter::WriteToFile(const char* filePath)
{
	std::ofstream outStream(filePath);

	if (outStream.is_open() == false)
		return false;

	YAML::Emitter out(outStream);

	out << YAML::BeginMap;

	int environmentId = world->GetEnvironmentId();
	if (environmentId >= 0)
	{
		const char* sourcePath = environmentManager->GetEnvironmentSourcePath(environmentId);
		out << YAML::Key << "environment" << YAML::Value << sourcePath;
	}

	out << YAML::Key << "objects" << YAML::Value << YAML::BeginSeq;
	for (Entity entity : *entityManager)
	{
		SceneObjectId sceneObj = world->Lookup(entity);
		if (sceneObj != SceneObjectId::Null && world->GetParent(sceneObj) == SceneObjectId::Null)
			WriteEntity(out, entity, sceneObj);
	}
	out << YAML::EndSeq; // objects


	out << YAML::EndMap;

	return true;
}

void LevelWriter::WriteEntity(YAML::Emitter& out, Entity entity, SceneObjectId sceneObj)
{
	out << YAML::BeginMap;

	const char* name = entityManager->GetDebugName(entity);
	if (name != nullptr)
		out << YAML::Key << "entity_name" << YAML::Value << name;

	out << YAML::Key << "components" << YAML::Value << YAML::BeginSeq;

	WriteTransformComponent(out, entity, sceneObj);
	WriteRenderComponent(out, entity);
	WriteLightComponent(out, entity);
	WriteCameraComponent(out, entity);

	out << YAML::EndSeq; // components

	SceneObjectId firstChild = world->GetFirstChild(sceneObj);
	if (firstChild != SceneObjectId::Null)
	{
		out << YAML::Key << "children" << YAML::Value << YAML::BeginSeq;

		SceneObjectId child = firstChild;
		while (child != SceneObjectId::Null)
		{
			Entity childEntity = world->GetEntity(child);

			WriteEntity(out, childEntity, child);

			child = world->GetNextSibling(child);
		}

		out << YAML::EndSeq;
	}

	out << YAML::EndMap;
}

void LevelWriter::WriteTransformComponent(YAML::Emitter& out, Entity entity, SceneObjectId sceneObj)
{
	const SceneEditTransform& transform = world->GetEditTransform(sceneObj);

	out << YAML::BeginMap;
	out << YAML::Key << ComponentTypeKey << YAML::Value << "transform";
	out << YAML::Key << "position" << YAML::Value << transform.translation;
	out << YAML::Key << "rotation" << YAML::Value << transform.rotation;
	out << YAML::Key << "scale" << YAML::Value << transform.scale;
	out << YAML::EndMap;
}

void LevelWriter::WriteRenderComponent(YAML::Emitter& out, Entity entity)
{
	RenderObjectId renderObj = renderer->Lookup(entity);
	if (renderObj != RenderObjectId::Null)
	{
		MeshId meshId = renderer->GetMeshId(renderObj);
		const char* meshPath = meshManager->GetPath(meshId);

		MaterialId materialId = renderer->GetOrderData(renderObj).material;
		const MaterialData& material = materialManager->GetMaterialData(materialId);
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
	CameraId cameraId = cameraSystem->Lookup(entity);
	if (cameraId != CameraId::Null)
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
