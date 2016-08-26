#pragma once

#include "Color.hpp"
#include "Skybox.hpp"

struct Mat4x4f;

class World
{
public:
	Color backgroundColor;
	Skybox skybox;
};
