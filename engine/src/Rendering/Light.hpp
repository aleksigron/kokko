#pragma once

enum class LightType
{
	Directional,
	Point,
	Spot
};

struct LightId
{
	unsigned int i;

	bool operator==(LightId other) { return i == other.i; }
	bool operator!=(LightId other) { return !operator==(other); }

	static const LightId Null;
};
