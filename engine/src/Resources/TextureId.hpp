#pragma once

struct TextureId
{
	unsigned int i;

	static TextureId Null;

	bool operator==(const TextureId& other) const { return i == other.i; }
	bool operator!=(const TextureId& other) const { return !operator==(other); }
};
