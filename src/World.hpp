#pragma once

#include <cstdint>

#include "Color.hpp"
#include "Skybox.hpp"

class World
{
private:
	Color backgroundColor;
	Skybox skybox;

	uint32_t skyboxMeshId;

public:
	World();
	~World();

	Color GetBackgroundColor() const { return backgroundColor; }
	void SetBackgroundColor(const Color& color) { backgroundColor = color; }
};
