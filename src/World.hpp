#pragma once

#include <cstdint>

#include "Color.hpp"
#include "ObjectId.hpp"

class ResourceManager;

class World
{
private:
	Color backgroundColor;

	unsigned int skyboxMaterialId;
	ObjectId skyboxMeshId;

public:
	World();
	~World();

	void InitializeSkyboxMesh(ResourceManager* resourceManager);
	ObjectId GetSkyboxMeshId() const { return skyboxMeshId; }

	void SetSkyboxMaterialId(uint32_t materialId) { skyboxMaterialId = materialId; }
	uint32_t GetSkyboxMaterialId() const { return skyboxMaterialId; }

	Color GetBackgroundColor() const { return backgroundColor; }
	void SetBackgroundColor(const Color& color) { backgroundColor = color; }
};
