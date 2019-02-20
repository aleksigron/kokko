#include "SceneManager.hpp"

#include <cstring>
#include <new>

#include "rapidjson/document.h"

#include "ValueSerialization.hpp"
#include "StringRef.hpp"

#include "Engine.hpp"
#include "Renderer.hpp"
#include "ResourceManager.hpp"
#include "Scene.hpp"
#include "File.hpp"

SceneManager::SceneManager() :
	scenes(nullptr),
	sceneCount(0),
	sceneAllocated(0),
	initialAllocation(4),
	primarySceneId(0)
{
}

SceneManager::~SceneManager()
{
}

void SceneManager::SetInitialAllocation(unsigned int allocationCount)
{
	this->initialAllocation = allocationCount;
}

void SceneManager::SetPrimarySceneId(unsigned int sceneId)
{
	this->primarySceneId = sceneId;
}

unsigned int SceneManager::GetPrimarySceneId() const
{
	return this->primarySceneId;
}

unsigned int SceneManager::LoadSceneFromFile(const char* path)
{
	unsigned int sceneId = 0;

	Buffer<char> sceneConfig = File::ReadText(path);

	if (sceneConfig.IsValid())
	{
		sceneId = this->CreateScene();

		if (sceneId != 0)
		{
			Scene* scene = this->GetScene(sceneId);

			Engine* engine = Engine::GetInstance();
			Renderer* renderer = engine->GetRenderer();
			ResourceManager* rm = engine->GetResourceManager();

			rapidjson::Document doc;
			doc.Parse(sceneConfig.Data(), sceneConfig.Count());

			if (doc.IsObject())
			{
				using MemberItr = rapidjson::Value::ConstMemberIterator;

				MemberItr objectsItr = doc.FindMember("objects");
				if (objectsItr != doc.MemberEnd() && objectsItr->value.IsArray())
				{
					rapidjson::Value::ConstValueIterator itr = objectsItr->value.Begin();
					rapidjson::Value::ConstValueIterator end = objectsItr->value.End();
					for (; itr != end; ++itr)
					{
						if (itr->IsObject())
						{
							RenderObject& renderObj = renderer->GetRenderObject(renderer->AddRenderObject());
							renderObj.sceneObjectId = scene->AddSceneObject();
							renderObj.layer = SceneLayer::World;

							MemberItr meshItr = itr->FindMember("mesh");
							if (meshItr != itr->MemberEnd() && meshItr->value.IsString())
							{
								StringRef path(meshItr->value.GetString(), meshItr->value.GetStringLength());
								renderObj.meshId = rm->CreateMeshFromFile(path);
							}

							MemberItr materialItr = itr->FindMember("material");
							if (materialItr != itr->MemberEnd() && materialItr->value.IsString())
							{
								StringRef path(materialItr->value.GetString(), materialItr->value.GetStringLength());
								renderObj.materialId = rm->CreateMaterialFromFile(path);
							}

							MemberItr positionItr = itr->FindMember("position");
							if (positionItr != itr->MemberEnd())
							{
								Vec3f pos = ValueSerialization::Deserialize_Vec3f(positionItr->value);
								scene->SetLocalTransform(renderObj.sceneObjectId, Mat4x4f::Translate(pos));
							}
						}
					}
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

					unsigned int materialId = rm->CreateMaterialFromFile(materialPath);

					scene->skybox.Initialize(scene, materialId);
				}
			}
		}
	}

	return sceneId;
}

unsigned int SceneManager::CreateScene()
{
	if (sceneCount == sceneAllocated)
	{
		unsigned int newAllocatedCount;
		if (sceneAllocated > 0)
			newAllocatedCount = sceneAllocated * 2;
		else
			newAllocatedCount = initialAllocation;

		void* buffer = operator new[](newAllocatedCount * sizeof(Scene));
		Scene* newScenes = static_cast<Scene*>(buffer);

		for (unsigned int i = 0; i < sceneCount; ++i)
		{
			newScenes[i] = std::move(this->scenes[i]);
			this->scenes[i].~Scene();
		}

		operator delete[](this->scenes); // Deleting a null pointer is a no-op
		this->scenes = newScenes;
		sceneAllocated = newAllocatedCount;
	}

	// Call constructor
	new(&scenes[sceneCount]) Scene();

	return ++sceneCount;
}

Scene* SceneManager::GetScene(unsigned int sceneId)
{
	if (sceneId > 0 && (sceneId - 1) < sceneCount)
		return &scenes[sceneId - 1];
	else
		return nullptr;
}
