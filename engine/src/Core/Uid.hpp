#pragma once

#include <cstdint>

class String;

namespace kokko
{

struct Uid
{
	uint64_t data[2];

	Uid() : data{ 0 }
	{
	}

	static Uid Create();

	bool operator==(const Uid& other)
	{
		return data[0] == other.data[0] && data[1] == other.data[1];
	}

	bool operator!=(const Uid& other)
	{
		return operator==(other) == false;
	}
};

}
