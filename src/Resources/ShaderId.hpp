#pragma once

struct ShaderId
{
	unsigned int i;

	bool IsNull() const { return i == 0; }

	bool operator==(const ShaderId& other) { return i == other.i; }
	bool operator!=(const ShaderId& other) { return operator==(other) == false; }
};
