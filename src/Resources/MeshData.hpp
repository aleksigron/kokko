#pragma once

struct MeshId
{
	unsigned int i;

	bool operator==(MeshId other) const { return other.i == i; }
	bool operator!=(MeshId other) const { return operator==(other) == false; }

	static const MeshId Null;
};
