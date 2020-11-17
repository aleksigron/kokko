#pragma once

enum class LightType {
	Directional,
	Point,
	Spot
};

struct LightId
{
	unsigned int i;

	bool IsNull() const { return i == 0; }
};
