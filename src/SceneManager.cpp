#include "SceneManager.hpp"

#include <cstring>
#include <new>

#include "Memory/Allocator.hpp"
#include "Engine.hpp"
#include "Scene.hpp"
#include "SceneLoader.hpp"
#include "StringRef.hpp"
#include "File.hpp"

SceneManager::SceneManager(Allocator* allocator) :
	allocator(allocator),
	scenes(nullptr),
	sceneCount(0),
	sceneAllocated(0),
	primarySceneId(0)
{
}

SceneManager::~SceneManager()
{
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
	unsigned int sceneId = 0;

	Buffer<char> sceneConfig = File::ReadText(path);

	if (sceneConfig.IsValid())
	{
		sceneId = this->CreateScene();

		if (sceneId != 0)
		{
			Scene* scene = this->GetScene(sceneId);

			SceneLoader loader(Engine::GetInstance(), scene);
			loader.Load(sceneConfig.GetRef());
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
