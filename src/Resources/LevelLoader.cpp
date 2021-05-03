#include "Resources/LevelLoader.hpp"

#include "rapidjson/document.h"

#include "yaml-cpp/yaml.h"

#include "Core/Core.hpp"

#include "Debug/LogHelper.hpp"

#include "Engine/Engine.hpp"

#include "Entity/EntityManager.hpp"

#include "Graphics/EnvironmentManager.hpp"
#include "Graphics/World.hpp"

#include "Rendering/CameraSystem.hpp"
#include "Rendering/Renderer.hpp"
#include "Rendering/LightManager.hpp"

#include "Resources/MeshManager.hpp"
#include "Resources/MaterialManager.hpp"
#include "Resources/ValueSerialization.hpp"
#include "Resources/YamlCustomTypes.hpp"

LevelLoader::LevelLoader(Engine* engine):
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

void LevelLoader::Load(BufferRef<char> sceneConfig)
{
	KOKKO_PROFILE_FUNCTION();

	YAML::Node node = YAML::Load(sceneConfig.data);

	const YAML::Node environment = node["environment"];
	if (environment.IsDefined() && environment.IsScalar())
	{
		int envId = environmentManager->LoadHdrEnvironmentMap(environment.Scalar().c_str());

		assert(envId >= 0);

		world->SetEnvironmentId(envId);
	}
	else
		world->SetEnvironmentId(-1);

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
					Log::Error("LevelLoader: Children were specified on an object with no transform component, ignoring children");
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

				default:
					Log::Error("LevelLoader: Unknown component type");
					break;
				}
			}
			else
				Log::Error("LevelLoader: Invalid component type");
		}
	}

	return createdTransform;
}

SceneObjectId LevelLoader::CreateTransformComponent(const YAML::Node& map, Entity entity, SceneObjectId parent)
{
	SceneObjectId sceneObject = world->AddSceneObject(entity);

	if (parent != SceneObjectId::Null)
		world->SetParent(sceneObject, parent);

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

	world->SetEditTransform(sceneObject, transform);

	return sceneObject;
}

void LevelLoader::CreateRenderComponent(const YAML::Node& map, Entity entity)
{
	YAML::Node meshNode = map["mesh"];
	YAML::Node materialNode = map["material"];

	if (meshNode.IsDefined() && meshNode.IsScalar() &&
		materialNode.IsDefined() && materialNode.IsScalar())
	{
		RenderObjectId renderObj = renderer->AddRenderObject(entity);

		const std::string& meshStr = meshNode.Scalar();
		StringRef meshPath(meshStr.data(), meshStr.size());
		MeshId meshId = meshManager->GetIdByPath(meshPath);

		assert(meshId != MeshId::Null);

		renderer->SetMeshId(renderObj, meshId);

		const std::string& materialStr = materialNode.Scalar();
		StringRef materialPath(materialStr.data(), materialStr.size());
		MaterialId materialId = materialManager->GetIdByPath(materialPath);

		assert(materialId != MaterialId::Null);

		RenderOrderData data;
		data.material = materialId;
		data.transparency = materialManager->GetMaterialData(materialId).transparency;

		renderer->SetOrderData(renderObj, data);
	}
}

void LevelLoader::CreateLightComponent(const YAML::Node& map, Entity entity)
{
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
			Log::Error("LevelLoader: Unknown light type");
			return;
		}
	}
	else
	{
		Log::Error("LevelLoader: Light type not specified");
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
			Log::Error("LevelLoader: Unknown projection type");
			return;
		}
	}
	else
	{
		Log::Error("LevelLoader: Projection type not specified");
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

	CameraId cameraId = cameraSystem->AddCameraComponent(entity);
	cameraSystem->SetProjectionParameters(cameraId, params);
}

void LevelLoader::LoadJson(BufferRef<char> sceneConfig)
{
	KOKKO_PROFILE_FUNCTION();

	rapidjson::Document doc;
	doc.Parse(sceneConfig.data, sceneConfig.count);

	if (doc.IsObject() == false)
		return;

	MemberItr envItr = doc.FindMember("environment");
	if (envItr != doc.MemberEnd() && envItr->value.IsString())
	{
		int envId = environmentManager->LoadHdrEnvironmentMap(envItr->value.GetString());

		assert(envId >= 0);

		world->SetEnvironmentId(envId);
	}
	else
		world->SetEnvironmentId(-1);

	MemberItr objectsItr = doc.FindMember("objects");
	if (objectsItr != doc.MemberEnd() && objectsItr->value.IsArray())
	{
		ValueItr itr = objectsItr->value.Begin();
		ValueItr end = objectsItr->value.End();

		CreateObjects(itr, end);
	}
}

void LevelLoader::CreateObjects(ValueItr itr, ValueItr end)
{
	for (; itr != end; ++itr)
	{
		if (itr->IsObject())
		{
			Entity entity = entityManager->Create();
			SceneObjectId sceneObj = world->AddSceneObject(entity);
			CreateSceneObject(itr, sceneObj);

			MemberItr componentsItr = itr->FindMember("components");
			if (componentsItr != itr->MemberEnd() && componentsItr->value.IsArray())
			{
				CreateComponents(componentsItr->value.Begin(), componentsItr->value.End(), entity);
			}
		}
	}
}

void LevelLoader::CreateChildObjects(ValueItr itr, ValueItr end, SceneObjectId parent)
{
	for (; itr != end; ++itr)
	{
		if (itr->IsObject())
		{
			Entity entity = entityManager->Create();
			SceneObjectId sceneObj = world->AddSceneObject(entity);
			world->SetParent(sceneObj, parent);
			CreateSceneObject(itr, sceneObj);

			MemberItr componentsItr = itr->FindMember("components");
			if (componentsItr != itr->MemberEnd() && componentsItr->value.IsArray())
			{
				CreateComponents(componentsItr->value.Begin(), componentsItr->value.End(), entity);
			}
		}
	}
}

void LevelLoader::CreateSceneObject(ValueItr itr, SceneObjectId sceneObject)
{
	SceneEditTransform transform;

	rapidjson::Value::ConstMemberIterator positionItr = itr->FindMember("position");
	if (positionItr != itr->MemberEnd())
		transform.translation = ValueSerialization::Deserialize_Vec3f(positionItr->value);

	rapidjson::Value::ConstMemberIterator rotItr = itr->FindMember("rotation");
	if (rotItr != itr->MemberEnd())
		transform.rotation = Math::DegreesToRadians(ValueSerialization::Deserialize_Vec3f(rotItr->value));

	rapidjson::Value::ConstMemberIterator scaleItr = itr->FindMember("scale");
	if (scaleItr != itr->MemberEnd())
		transform.scale = ValueSerialization::Deserialize_Vec3f(scaleItr->value);
	else
		transform.scale = Vec3f(1.0f, 1.0f, 1.0f);

	world->SetEditTransform(sceneObject, transform);

	rapidjson::Value::ConstMemberIterator childrenItr = itr->FindMember("children");
	if (childrenItr != itr->MemberEnd() && childrenItr->value.IsArray())
	{
		rapidjson::Value::ConstValueIterator itr = childrenItr->value.Begin();
		rapidjson::Value::ConstValueIterator end = childrenItr->value.End();

		CreateChildObjects(itr, end, sceneObject);
	}
}

void LevelLoader::CreateComponents(ValueItr itr, ValueItr end, Entity entity)
{
	for (; itr != end; ++itr)
	{
		if (itr->IsObject())
		{
			MemberItr typeItr = itr->FindMember("type");
			if (typeItr != itr->MemberEnd() && typeItr->value.IsString())
			{
				StringRef type(typeItr->value.GetString(), typeItr->value.GetStringLength());
				uint32_t typeHash = Hash::FNV1a_32(type.str, type.len);

				switch (typeHash)
				{
				case "renderObject"_hash:
					CreateRenderObject(itr, entity);
					break;

				case "light"_hash:
					CreateLight(itr, entity);
					break;

				default:
					Log::Error("LevelLoader: Unknown component type");
					break;
				}
			}
			else
				Log::Error("LevelLoader: Invalid component type");
		}
	}
}

void LevelLoader::CreateRenderObject(ValueItr itr, Entity entity)
{
	MemberItr meshItr = itr->FindMember("mesh");
	MemberItr materialItr = itr->FindMember("material");

	if (meshItr != itr->MemberEnd() && meshItr->value.IsString() &&
		materialItr != itr->MemberEnd() && materialItr->value.IsString())
	{
		RenderObjectId renderObj = renderer->AddRenderObject(entity);

		StringRef meshPath(meshItr->value.GetString(), meshItr->value.GetStringLength());
		MeshId meshId = meshManager->GetIdByPath(meshPath);
		renderer->SetMeshId(renderObj, meshId);

		StringRef matPath(materialItr->value.GetString(), materialItr->value.GetStringLength());
		MaterialId matId = materialManager->GetIdByPath(matPath);

		assert(matId != MaterialId::Null);

		RenderOrderData data;
		data.material = matId;
		data.transparency = materialManager->GetMaterialData(matId).transparency;

		renderer->SetOrderData(renderObj, data);
	}
}

void LevelLoader::CreateLight(ValueItr itr, Entity entity)
{
	LightType type;

	MemberItr typeItr = itr->FindMember("lightType");
	if (typeItr == itr->MemberEnd() || typeItr->value.IsString() == false)
	{
		Log::Error("LevelLoader: Invalid light type");
		return;	
	}

	StringRef typeStr(typeItr->value.GetString(), typeItr->value.GetStringLength());
	uint32_t typeHash = Hash::FNV1a_32(typeStr.str, typeStr.len);

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
		Log::Error("LevelLoader: Unknown light type");
		return;
	}

	Vec3f color(1.0f, 1.0f, 1.0f);
	MemberItr colorItr = itr->FindMember("color");
	if (colorItr != itr->MemberEnd())
	{
		color = ValueSerialization::Deserialize_Vec3f(colorItr->value);
	}

	bool shadowCasting = false;
	MemberItr shadowItr = itr->FindMember("shadowCasting");
	if (shadowItr != itr->MemberEnd() && shadowItr->value.IsBool())
	{
		shadowCasting = shadowItr->value.GetBool();
	}

	float radius = -1.0f;
	MemberItr radiusItr = itr->FindMember("radius");
	if (radiusItr != itr->MemberEnd() && radiusItr->value.IsNumber())
	{
		radius = radiusItr->value.GetFloat();
	}

	float angle = 0.0f;
	MemberItr angleItr = itr->FindMember("angle");
	if (angleItr != itr->MemberEnd() && angleItr->value.IsNumber())
	{
		angle = angleItr->value.GetFloat();
	}

	LightId lightId = lightManager->AddLight(entity);
	lightManager->SetLightType(lightId, type);
	lightManager->SetColor(lightId, color);
	lightManager->SetShadowCasting(lightId, shadowCasting);

	if (type != LightType::Directional)
	{
		if (radius > 0.0f)
			lightManager->SetRadius(lightId, radius);
		else
			lightManager->SetRadiusFromColor(lightId);
	}

	if (type == LightType::Spot)
		lightManager->SetSpotAngle(lightId, Math::DegreesToRadians(angle));
}
