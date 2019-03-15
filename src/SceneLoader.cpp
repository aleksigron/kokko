#include "SceneLoader.hpp"

#include "rapidjson/document.h"

#include "Engine.hpp"
#include "Renderer.hpp"
#include "MeshManager.hpp"
#include "EntityManager.hpp"
#include "ResourceManager.hpp"
#include "ValueSerialization.hpp"

SceneLoader::SceneLoader(Engine* engine, Scene* scene):
	scene(scene),
	renderer(engine->GetRenderer()),
	meshManager(engine->GetMeshManager()),
	entityManager(engine->GetEntityManager()),
	resourceManager(engine->GetResourceManager())
{
}

void SceneLoader::Load(BufferRef<char> sceneConfig)
{
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

	MemberItr colorItr = doc.FindMember("background-color");
	if (colorItr != doc.MemberEnd())
	{
		scene->backgroundColor = ValueSerialization::Deserialize_Color(colorItr->value);
	}

	MemberItr skyboxItr = doc.FindMember("skybox-material");
	if (skyboxItr != doc.MemberEnd() && skyboxItr->value.IsString())
	{
		StringRef materialPath(skyboxItr->value.GetString(), skyboxItr->value.GetStringLength());

		unsigned int materialId = resourceManager->CreateMaterialFromFile(materialPath);

		scene->skybox.Initialize(scene, materialId);
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
			CreateRenderObject(itr, entity);
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
			CreateRenderObject(itr, entity);
		}
	}
}

void SceneLoader::CreateSceneObject(ValueItr itr, SceneObjectId sceneObject)
{
	rapidjson::Value::ConstMemberIterator positionItr = itr->FindMember("position");
	if (positionItr != itr->MemberEnd())
	{
		Vec3f pos = ValueSerialization::Deserialize_Vec3f(positionItr->value);
		scene->SetLocalTransform(sceneObject, Mat4x4f::Translate(pos));
	}

	rapidjson::Value::ConstMemberIterator childrenItr = itr->FindMember("children");
	if (childrenItr != itr->MemberEnd() && childrenItr->value.IsArray())
	{
		rapidjson::Value::ConstValueIterator itr = childrenItr->value.Begin();
		rapidjson::Value::ConstValueIterator end = childrenItr->value.End();

		CreateChildObjects(itr, end, sceneObject);
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
		renderer->SetSceneLayer(renderObj, SceneLayer::World);

		StringRef meshPath(meshItr->value.GetString(), meshItr->value.GetStringLength());
		MeshId meshId = meshManager->GetIdByPath(meshPath);
		if (meshId.IsValid())
			renderer->SetMeshId(renderObj, meshId);

		StringRef matPath(materialItr->value.GetString(), materialItr->value.GetStringLength());
		unsigned int materialId = resourceManager->CreateMaterialFromFile(matPath);
		if (materialId != 0)
			renderer->SetMaterialId(renderObj, materialId);
	}
}
