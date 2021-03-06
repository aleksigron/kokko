#include "Resources/LevelLoader.hpp"

#include "yaml-cpp/yaml.h"

#include "Core/Core.hpp"

#include "Engine/World.hpp"

#include "Engine/EntityManager.hpp"

#include "Graphics/EnvironmentManager.hpp"
#include "Graphics/Scene.hpp"
#include "Graphics/TerrainSystem.hpp"

#include "Rendering/CameraSystem.hpp"
#include "Rendering/Renderer.hpp"
#include "Rendering/LightManager.hpp"

#include "Resources/MeshManager.hpp"
#include "Resources/MaterialManager.hpp"
#include "Resources/ValueSerialization.hpp"
#include "Resources/YamlCustomTypes.hpp"

LevelLoader::LevelLoader(World* world, const ResourceManagers& resManagers):
	world(world),
	resourceManagers(resManagers)
{
}

void LevelLoader::Load(const char* data)
{
	KOKKO_PROFILE_FUNCTION();

	Scene* scene = world->GetScene();

	YAML::Node node = YAML::Load(data);

	const YAML::Node environment = node["environment"];
	if (environment.IsDefined() && environment.IsScalar())
	{
		int envId = resourceManagers.environmentManager->LoadHdrEnvironmentMap(environment.Scalar().c_str());

		assert(envId >= 0);

		scene->SetEnvironmentId(envId);
	}
	else
		scene->SetEnvironmentId(-1);

	const YAML::Node objects = node["objects"];
	if (objects.IsDefined() && objects.IsSequence())
	{
		CreateObjects(objects, SceneObjectId::Null);
	}
}

void LevelLoader::CreateObjects(const YAML::Node& childSequence, SceneObjectId parent)
{
	YAML::const_iterator itr = childSequence.begin();
	YAML::const_iterator end = childSequence.end();

	for (; itr != end; ++itr)
	{
		if (itr->IsMap())
		{
			EntityManager* entityManager = world->GetEntityManager();
			Entity entity = entityManager->Create();

			SceneObjectId createdTransform = SceneObjectId::Null;

			const YAML::Node entityNameNode = (*itr)["entity_name"];
			if (entityNameNode.IsDefined() && entityNameNode.IsScalar())
			{
				const std::string& nameStr = entityNameNode.Scalar();
				entityManager->SetDebugName(entity, nameStr.c_str());
			}

			const YAML::Node componentsNode = (*itr)["components"];
			if (componentsNode.IsDefined() && componentsNode.IsSequence())
			{
				createdTransform = CreateComponents(componentsNode, entity, parent);
			}

			const YAML::Node childrenNode = (*itr)["children"];
			if (childrenNode.IsDefined() && childrenNode.IsSequence())
			{
				if (createdTransform != SceneObjectId::Null)
					CreateObjects(childrenNode, createdTransform);
				else
					KK_LOG_ERROR("LevelLoader: Children were specified on an object with no transform component, ignoring children");
			}
		}
	}
}

SceneObjectId LevelLoader::CreateComponents(const YAML::Node& componentSequence, Entity entity, SceneObjectId parent)
{
	YAML::const_iterator itr = componentSequence.begin();
	YAML::const_iterator end = componentSequence.end();

	SceneObjectId createdTransform = SceneObjectId::Null;

	for (; itr != end; ++itr)
	{
		if (itr->IsMap())
		{
			YAML::Node typeNode = (*itr)["component_type"];
			if (typeNode.IsDefined() && typeNode.IsScalar())
			{
				const std::string& typeStr = typeNode.Scalar();
				uint32_t typeHash = Hash::FNV1a_32(typeStr.data(), typeStr.size());

				switch (typeHash)
				{
				case "transform"_hash:
					createdTransform = CreateTransformComponent(*itr, entity, parent);
					break;

				case "render"_hash:
					CreateRenderComponent(*itr, entity);
					break;

				case "light"_hash:
					CreateLightComponent(*itr, entity);
					break;

				case "camera"_hash:
					CreateCameraComponent(*itr, entity);
					break;

				case "terrain"_hash:
					CreateTerrainComponent(*itr, entity);

				default:
					KK_LOG_ERROR("LevelLoader: Unknown component type");
					break;
				}
			}
			else
				KK_LOG_ERROR("LevelLoader: Invalid component type");
		}
	}

	return createdTransform;
}

SceneObjectId LevelLoader::CreateTransformComponent(const YAML::Node& map, Entity entity, SceneObjectId parent)
{
	Scene* scene = world->GetScene();
	SceneObjectId sceneObject = scene->AddSceneObject(entity);

	if (parent != SceneObjectId::Null)
		scene->SetParent(sceneObject, parent);

	SceneEditTransform transform;

	YAML::Node positionNode = map["position"];
	if (positionNode.IsDefined() && positionNode.IsSequence())
		transform.translation = positionNode.as<Vec3f>();

	YAML::Node rotationNode = map["rotation"];
	if (rotationNode.IsDefined() && rotationNode.IsSequence())
		transform.rotation = rotationNode.as<Vec3f>();

	YAML::Node scaleNode = map["scale"];
	if (scaleNode.IsDefined() && scaleNode.IsSequence())
		transform.scale = scaleNode.as<Vec3f>();

	scene->SetEditTransform(sceneObject, transform);

	return sceneObject;
}

void LevelLoader::CreateRenderComponent(const YAML::Node& map, Entity entity)
{
	Renderer* renderer = world->GetRenderer();

	YAML::Node meshNode = map["mesh"];
	YAML::Node materialNode = map["material"];

	if (meshNode.IsDefined() && meshNode.IsScalar() &&
		materialNode.IsDefined() && materialNode.IsScalar())
	{
		RenderObjectId renderObj = renderer->AddRenderObject(entity);

		const std::string& meshStr = meshNode.Scalar();
		StringRef meshPath(meshStr.data(), meshStr.size());
		MeshId meshId = resourceManagers.meshManager->GetIdByPath(meshPath);

		assert(meshId != MeshId::Null);

		renderer->SetMeshId(renderObj, meshId);

		const std::string& materialStr = materialNode.Scalar();
		StringRef materialPath(materialStr.data(), materialStr.size());
		MaterialId materialId = resourceManagers.materialManager->GetIdByPath(materialPath);

		assert(materialId != MaterialId::Null);

		RenderOrderData data;
		data.material = materialId;
		data.transparency = resourceManagers.materialManager->GetMaterialData(materialId).transparency;

		renderer->SetOrderData(renderObj, data);
	}
}

void LevelLoader::CreateLightComponent(const YAML::Node& map, Entity entity)
{
	LightManager* lightManager = world->GetLightManager();

	LightType type;

	YAML::Node typeNode = map["type"];
	if (typeNode.IsDefined() && typeNode.IsScalar())
	{
		const std::string& typeStr = typeNode.Scalar();
		uint32_t typeHash = Hash::FNV1a_32(typeStr.data(), typeStr.size());

		switch (typeHash)
		{
		case "directional"_hash:
			type = LightType::Directional;
			break;

		case "point"_hash:
			type = LightType::Point;
			break;

		case "spot"_hash:
			type = LightType::Spot;
			break;

		default:
			KK_LOG_ERROR("LevelLoader: Unknown light type");
			return;
		}
	}
	else
	{
		KK_LOG_ERROR("LevelLoader: Light type not specified");
		return;
	}


	Vec3f color(1.0f, 1.0f, 1.0f);
	YAML::Node colorNode = map["color"];
	if (colorNode.IsDefined() && colorNode.IsSequence())
		color = colorNode.as<Vec3f>();

	float radius = -1.0f;
	YAML::Node radiusNode = map["radius"];
	if (radiusNode.IsDefined() && radiusNode.IsScalar())
		radius = radiusNode.as<float>();

	float angle = 1.0f;
	YAML::Node angleNode = map["spot_angle"];
	if (angleNode.IsDefined() && angleNode.IsScalar())
		angle = angleNode.as<float>();

	bool shadowCasting = false;
	YAML::Node shadowNode = map["cast_shadow"];
	if (shadowNode.IsDefined() && shadowNode.IsScalar())
		shadowCasting = shadowNode.as<bool>();

	LightId lightId = lightManager->AddLight(entity);
	lightManager->SetLightType(lightId, type);
	lightManager->SetColor(lightId, color);

	if (type != LightType::Directional)
	{
		if (radius > 0.0f)
			lightManager->SetRadius(lightId, radius);
		else
			lightManager->SetRadiusFromColor(lightId);
	}
	else
		lightManager->SetRadius(lightId, 1.0f);

	lightManager->SetSpotAngle(lightId, angle);
	lightManager->SetShadowCasting(lightId, shadowCasting);
}

void LevelLoader::CreateCameraComponent(const YAML::Node& map, Entity entity)
{
	CameraSystem* cameraSystem = world->GetCameraSystem();

	ProjectionParameters params;
	params.projection = ProjectionType::Perspective;
	params.perspectiveFieldOfView = 1.0f;
	params.perspectiveNear = 0.1f;
	params.perspectiveFar = 100.0f;
	params.orthographicHeight = 2.0f;
	params.orthographicNear = 0.0f;
	params.orthographicFar = 10.0f;
	params.SetAspectRatio(16.0f, 9.0f);

	YAML::Node typeNode = map["projection_type"];
	if (typeNode.IsDefined() && typeNode.IsScalar())
	{
		const std::string& typeStr = typeNode.Scalar();
		uint32_t typeHash = Hash::FNV1a_32(typeStr.data(), typeStr.size());

		switch (typeHash)
		{
		case "perspective"_hash:
			params.projection = ProjectionType::Perspective;
			break;

		case "orthographic"_hash:
			params.projection = ProjectionType::Orthographic;
			break;

		default:
			KK_LOG_ERROR("LevelLoader: Unknown projection type");
			return;
		}
	}
	else
	{
		KK_LOG_ERROR("LevelLoader: Projection type not specified");
		return;
	}

	YAML::Node fovNode = map["field_of_view"];
	if (fovNode.IsDefined() && fovNode.IsScalar())
		params.perspectiveFieldOfView = fovNode.as<float>();

	YAML::Node heightNode = map["orthographic_height"];
	if (heightNode.IsDefined() && heightNode.IsScalar())
		params.orthographicHeight = heightNode.as<float>();

	YAML::Node nearNode = map["near"];
	if (nearNode.IsDefined() && nearNode.IsScalar())
	{
		float near = nearNode.as<float>();

		if (params.projection == ProjectionType::Perspective)
			params.perspectiveNear = near;
		else
			params.orthographicNear = near;
	}

	YAML::Node farNode = map["far"];
	if (farNode.IsDefined() && farNode.IsScalar())
	{
		float far = farNode.as<float>();

		if (params.projection == ProjectionType::Perspective)
			params.perspectiveFar = far;
		else
			params.orthographicFar = far;
	}

	CameraId cameraId = cameraSystem->AddComponentToEntity(entity);
	cameraSystem->SetData(cameraId, params);
}

void LevelLoader::CreateTerrainComponent(const YAML::Node& map, Entity entity)
{
	TerrainSystem* terrainSystem = world->GetTerrainSystem();

	TerrainInstance terrain;

	YAML::Node sizeNode = map["terrain_size"];
	if (sizeNode.IsDefined() && sizeNode.IsScalar())
		terrain.terrainSize = sizeNode.as<float>();

	YAML::Node resNode = map["terrain_resolution"];
	if (resNode.IsDefined() && resNode.IsScalar())
		terrain.terrainResolution = resNode.as<int>();

	YAML::Node scaleNode = map["texture_scale"];
	if (scaleNode.IsDefined() && scaleNode.IsSequence())
		terrain.textureScale = scaleNode.as<Vec2f>();

	YAML::Node minNode = map["min_height"];
	if (minNode.IsDefined() && minNode.IsScalar())
		terrain.minHeight = minNode.as<float>();

	YAML::Node maxNode = map["max_height"];
	if (maxNode.IsDefined() && maxNode.IsScalar())
		terrain.maxHeight = maxNode.as<float>();

	TerrainId id = terrainSystem->AddComponentToEntity(entity);
	terrainSystem->SetData(id, terrain);
	terrainSystem->InitializeTerrain(id);
}
