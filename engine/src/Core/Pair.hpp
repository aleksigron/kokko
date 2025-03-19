#pragma once

namespace kokko
{

template <typename T1, typename T2>
struct Pair
{
	T1 first;
	T2 second;

	Pair() = default;
	Pair(const Pair&) = default;
	Pair(Pair&&) = default;

	Pair(const T1& first, const T2& second) :
		first(first),
		second(second)
	{
	}

	Pair& operator=(const Pair& other) = default;
	Pair& operator=(Pair&& other) = default;
};

} // namespace kokko
