#pragma once

#include "Vec3.hpp"
#include "Entity.hpp"

class Scene;

class Skybox
{
private:
	unsigned int renderSceneId;

	unsigned int renderObjectId;
	Entity entity;

public:
	Skybox();
	~Skybox();

	bool IsInitialized() const { return renderSceneId != 0; }

	void Initialize(Scene* scene, unsigned int materialId);
	void UpdateTransform(const Vec3f& cameraPosition) const;
};
