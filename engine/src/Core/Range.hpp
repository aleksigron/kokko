#pragma once

template <typename T>
struct Range
{
	T start;
	T end;

	Range() : start(0), end(0)
	{
	}

	Range(T start, T end) : start(start), end(end)
	{
	}
};
