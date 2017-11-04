#pragma once

class ResourceManager;
class Scene;

/**
 * SceneManager manages scenes. Scenes are identified by an ID of type
 * <unsigned int>. ID with value 0 signifies invalid or null.
 */
class SceneManager
{
private:
	ResourceManager* resourceManager;

	Scene* scenes;
	unsigned int sceneCount;
	unsigned int sceneAllocated;
	unsigned int initialAllocation;

	unsigned int primarySceneId;
	
public:
	SceneManager(ResourceManager* resourceManager);
	~SceneManager();

	void SetInitialAllocation(unsigned int allocationCount);

	void SetPrimarySceneId(unsigned int sceneId);
	unsigned int GetPrimarySceneId() const;

	unsigned int LoadSceneFromFile(const char* path);
	unsigned int CreateScene();
	Scene* GetScene(unsigned int sceneId);
};
