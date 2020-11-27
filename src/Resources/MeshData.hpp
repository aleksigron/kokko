#pragma once

struct MeshId
{
	unsigned int i;

	bool IsValid() const { return i != 0; }
};
