#pragma once

#include <cstdint>

namespace kokko
{

struct MaterialId
{
	uint16_t i;

	bool operator==(MaterialId other) { return i == other.i; }
	bool operator!=(MaterialId other) { return operator==(other) == false; }

	static const MaterialId Null;
};

} // namespace kokko
