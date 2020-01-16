#pragma once

#include "Math/Vec3.hpp"
#include "Entity.hpp"
#include "MaterialData.hpp"

class Scene;

class Skybox
{
private:
	unsigned int renderSceneId;

	Entity entity;

public:
	Skybox();
	~Skybox();

	bool IsInitialized() const { return renderSceneId != 0; }

	void Initialize(Scene* scene, MaterialId materialId);
	void UpdateTransform(const Vec3f& cameraPosition) const;
};
