#pragma once

struct MaterialId
{
	unsigned int i;

	bool operator==(MaterialId other) { return i == other.i; }
	bool operator!=(MaterialId other) { return operator==(other) == false; }

	static const MaterialId Null;
};
