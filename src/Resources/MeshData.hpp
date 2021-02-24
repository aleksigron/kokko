#pragma once

struct MeshId
{
	unsigned int i;

	bool IsValid() const { return i != 0; }

	bool operator==(MeshId other) const { return other.i == i; }
	bool operator!=(MeshId other) const { return operator==(other) == false; }
};
