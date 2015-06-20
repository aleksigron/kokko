#pragma once

template <typename T>
class Vec2
{
public:
	T x;
	T y;
	
	Vec2(): x(0), y(0) {}
	Vec2(T x, T y): x(x), y(y) {}
};

using Vec2i = Vec2<int>;
using Vec2f = Vec2<float>;
using Vec2d = Vec2<double>;