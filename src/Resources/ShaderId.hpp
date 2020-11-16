#pragma once

struct ShaderId
{
	unsigned int i;

	bool IsNull() const { return i == 0; }
};
