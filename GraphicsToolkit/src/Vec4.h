#pragma once

#include <cmath>

template <typename T>
struct Vec4
{
	T x, y, z, w;
	
	Vec4(): x(0), y(0), z(0), w(0) {}
	Vec4(T x, T y, T z, T w): x(x), y(y), z(z), w(w) {}
};

using Vec4i = Vec4<int>;
using Vec4f = Vec4<float>;
using Vec4d = Vec4<double>;