#include "Scene/SceneLoader.hpp"

#include "rapidjson/document.h"

#include "Core/Core.hpp"

#include "Debug/LogHelper.hpp"

#include "Engine/Engine.hpp"

#include "Entity/EntityManager.hpp"

#include "Graphics/EnvironmentManager.hpp"

#include "Rendering/Renderer.hpp"
#include "Rendering/LightManager.hpp"

#include "Resources/MeshManager.hpp"
#include "Resources/MaterialManager.hpp"
#include "Resources/ValueSerialization.hpp"

#include "Scene/Scene.hpp"

SceneLoader::SceneLoader(Engine* engine, Scene* scene):
	scene(scene),
	renderer(engine->GetRenderer()),
	meshManager(engine->GetMeshManager()),
	materialManager(engine->GetMaterialManager()),
	entityManager(engine->GetEntityManager()),
	lightManager(engine->GetLightManager()),
	environmentManager(engine->GetEnvironmentManager())
{
}

void SceneLoader::Load(BufferRef<char> sceneConfig)
{
	KOKKO_PROFILE_FUNCTION();

	rapidjson::Document doc;
	doc.Parse(sceneConfig.data, sceneConfig.count);

	if (doc.IsObject() == false)
		return;

	MemberItr objectsItr = doc.FindMember("objects");
	if (objectsItr != doc.MemberEnd() && objectsItr->value.IsArray())
	{
		ValueItr itr = objectsItr->value.Begin();
		ValueItr end = objectsItr->value.End();

		CreateObjects(itr, end);
	}

	MemberItr envItr = doc.FindMember("environment");
	if (envItr != doc.MemberEnd() && envItr->value.IsString())
	{
		int envId = environmentManager->LoadHdrEnvironmentMap(envItr->value.GetString());

		assert(envId >= 0);

		scene->SetEnvironmentId(envId);
	}

	MemberItr colorItr = doc.FindMember("ambient-color");
	if (colorItr != doc.MemberEnd())
	{
		scene->ambientColor = ValueSerialization::Deserialize_Color(colorItr->value);
	}

	MemberItr skyboxItr = doc.FindMember("skybox-material");
	if (skyboxItr != doc.MemberEnd() && skyboxItr->value.IsString())
	{
		StringRef materialPath(skyboxItr->value.GetString(), skyboxItr->value.GetStringLength());

		MaterialId matId = materialManager->GetIdByPath(materialPath);
		
		assert(matId.IsNull() == false);

		scene->SetSkyboxMaterial(matId);
	}
}

void SceneLoader::CreateObjects(ValueItr itr, ValueItr end)
{
	for (; itr != end; ++itr)
	{
		if (itr->IsObject())
		{
			Entity entity = entityManager->Create();
			SceneObjectId sceneObj = scene->AddSceneObject(entity);
			CreateSceneObject(itr, sceneObj);

			MemberItr componentsItr = itr->FindMember("components");
			if (componentsItr != itr->MemberEnd() && componentsItr->value.IsArray())
			{
				CreateComponents(componentsItr->value.Begin(), componentsItr->value.End(), entity);
			}
		}
	}
}

void SceneLoader::CreateChildObjects(ValueItr itr, ValueItr end, SceneObjectId parent)
{
	for (; itr != end; ++itr)
	{
		if (itr->IsObject())
		{
			Entity entity = entityManager->Create();
			SceneObjectId sceneObj = scene->AddSceneObject(entity);
			scene->SetParent(sceneObj, parent);
			CreateSceneObject(itr, sceneObj);

			MemberItr componentsItr = itr->FindMember("components");
			if (componentsItr != itr->MemberEnd() && componentsItr->value.IsArray())
			{
				CreateComponents(componentsItr->value.Begin(), componentsItr->value.End(), entity);
			}
		}
	}
}

void SceneLoader::CreateSceneObject(ValueItr itr, SceneObjectId sceneObject)
{
	Mat4x4f transform;

	rapidjson::Value::ConstMemberIterator scaleItr = itr->FindMember("scale");
	if (scaleItr != itr->MemberEnd())
	{
		Vec3f scale = ValueSerialization::Deserialize_Vec3f(scaleItr->value);
		transform = Mat4x4f::Scale(scale) * transform;
	}

	rapidjson::Value::ConstMemberIterator rotItr = itr->FindMember("rotation");
	if (rotItr != itr->MemberEnd())
	{
		Vec3f rot = ValueSerialization::Deserialize_Vec3f(rotItr->value);
		transform = Mat4x4f::RotateEuler(Math::DegreesToRadians(rot)) * transform;
	}

	rapidjson::Value::ConstMemberIterator positionItr = itr->FindMember("position");
	if (positionItr != itr->MemberEnd())
	{
		Vec3f pos = ValueSerialization::Deserialize_Vec3f(positionItr->value);
		transform = Mat4x4f::Translate(pos) * transform;
	}

	scene->SetLocalTransform(sceneObject, transform);

	rapidjson::Value::ConstMemberIterator childrenItr = itr->FindMember("children");
	if (childrenItr != itr->MemberEnd() && childrenItr->value.IsArray())
	{
		rapidjson::Value::ConstValueIterator itr = childrenItr->value.Begin();
		rapidjson::Value::ConstValueIterator end = childrenItr->value.End();

		CreateChildObjects(itr, end, sceneObject);
	}
}

void SceneLoader::CreateComponents(ValueItr itr, ValueItr end, Entity entity)
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
					Log::Error("SceneLoader: Unknown component type");
					break;
				}
			}
			else
				Log::Error("SceneLoader: Invalid component type");
		}
	}
}

void SceneLoader::CreateRenderObject(ValueItr itr, Entity entity)
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

		assert(matId.IsNull() == false);

		RenderOrderData data;
		data.material = matId;
		data.transparency = materialManager->GetMaterialData(matId).transparency;

		renderer->SetOrderData(renderObj, data);
	}
}

void SceneLoader::CreateLight(ValueItr itr, Entity entity)
{
	LightType type;

	MemberItr typeItr = itr->FindMember("lightType");
	if (typeItr == itr->MemberEnd() || typeItr->value.IsString() == false)
	{
		Log::Error("SceneLoader: Invalid light type");
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
		Log::Error("SceneLoader: Unknown light type");
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
