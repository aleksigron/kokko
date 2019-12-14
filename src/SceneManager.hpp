#pragma once

#include "StringRef.hpp"

class Allocator;
class Scene;

/**
 * SceneManager manages scenes. Scenes are identified by an ID of type
 * <unsigned int>. ID with value 0 signifies invalid or null.
 */
class SceneManager
{
private:
	Allocator* allocator;

	Scene* scenes;
	unsigned int sceneCount;
	unsigned int sceneAllocated;

	unsigned int primarySceneId;
	
public:
	SceneManager(Allocator* allocator);
	~SceneManager();

	void SetPrimarySceneId(unsigned int sceneId);
	unsigned int GetPrimarySceneId() const;

	unsigned int LoadSceneFromFile(StringRef path);
	unsigned int CreateScene();
	Scene* GetScene(unsigned int sceneId);
};
