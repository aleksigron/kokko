#pragma once

#include "Vec3.hpp"
#include "Entity.hpp"

class Scene;
struct Material;

class Skybox
{
private:
	unsigned int renderSceneId;

	Entity entity;

public:
	Skybox();
	~Skybox();

	bool IsInitialized() const { return renderSceneId != 0; }

	void Initialize(Scene* scene, const Material& material);
	void UpdateTransform(const Vec3f& cameraPosition) const;
};
