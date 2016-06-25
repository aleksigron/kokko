#pragma once

#include "Color.hpp"
#include "Skybox.hpp"
#include "ObjectId.hpp"

class ResourceManager;

class World
{
private:
	Color backgroundColor;
	Skybox skybox;

	ObjectId skyboxMeshId;

public:
	World();
	~World();

	void InitializeSkyboxMesh(ResourceManager* resourceManager);

	Color GetBackgroundColor() const { return backgroundColor; }
	void SetBackgroundColor(const Color& color) { backgroundColor = color; }
};
