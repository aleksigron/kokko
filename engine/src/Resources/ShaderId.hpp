#pragma once

namespace kokko
{

struct ShaderId
{
	unsigned int i;

	bool operator==(const ShaderId& other) { return i == other.i; }
	bool operator!=(const ShaderId& other) { return operator==(other) == false; }

	static const ShaderId Null;
};

} // namespace kokko
