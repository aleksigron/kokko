#include "Scene/SceneManager.hpp"

#include <cstring>
#include <new>

#include "Core/Core.hpp"
#include "Core/String.hpp"
#include "Core/StringRef.hpp"

#include "Engine/Engine.hpp"

#include "Memory/Allocator.hpp"

#include "Scene/Scene.hpp"
#include "Scene/SceneLoader.hpp"

#include "System/File.hpp"

SceneManager::SceneManager(Engine* engine, Allocator* allocator) :
	engine(engine),
	allocator(allocator),
	scenes(nullptr),
	sceneCount(0),
	sceneAllocated(0),
	primarySceneId(0)
{
}

SceneManager::~SceneManager()
{
	for (unsigned int i = 0; i < sceneCount; ++i)
		scenes[i].~Scene();

	allocator->Deallocate(scenes);
}

void SceneManager::SetPrimarySceneId(unsigned int sceneId)
{
	this->primarySceneId = sceneId;
}

unsigned int SceneManager::GetPrimarySceneId() const
{
	return this->primarySceneId;
}

unsigned int SceneManager::LoadSceneFromFile(StringRef path)
{
	KOKKO_PROFILE_FUNCTION();

	unsigned int sceneId = 0;

	Buffer<char> sceneConfig(allocator);
	String pathStr(allocator, path);

	if (File::ReadText(pathStr.GetCStr(), sceneConfig))
	{
		sceneId = this->CreateScene();

		if (sceneId != 0)
		{
			Scene* scene = this->GetScene(sceneId);

			SceneLoader loader(engine, scene);
			loader.Load(sceneConfig.GetRef());
		}
	}

	return sceneId;
}

unsigned int SceneManager::CreateScene()
{
	KOKKO_PROFILE_FUNCTION();

	if (sceneCount == sceneAllocated)
	{
		unsigned int newAllocatedCount;
		if (sceneAllocated > 0)
			newAllocatedCount = sceneAllocated * 2;
		else
			newAllocatedCount = 4;

		void* buffer = allocator->Allocate(newAllocatedCount * sizeof(Scene));
		Scene* newScenes = static_cast<Scene*>(buffer);

		for (unsigned int i = 0; i < sceneCount; ++i)
		{
			newScenes[i] = std::move(this->scenes[i]);
			this->scenes[i].~Scene();
		}

		allocator->Deallocate(this->scenes);
		this->scenes = newScenes;
		sceneAllocated = newAllocatedCount;
	}

	// Call constructor
	new(&scenes[sceneCount]) Scene(allocator, sceneCount + 1);

	return ++sceneCount;
}

Scene* SceneManager::GetScene(unsigned int sceneId)
{
	if (sceneId > 0 && (sceneId - 1) < sceneCount)
		return &scenes[sceneId - 1];
	else
		return nullptr;
}
