#pragma once

struct MaterialId
{
	unsigned int i;

	bool IsNull() const { return i == 0; }

	bool operator==(MaterialId other) { return i == other.i; }
	bool operator!=(MaterialId other) { return operator==(other) == false; }
};
