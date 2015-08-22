#pragma once

#include "Vec3.h"

template <typename T>
struct Vec4
{
	T x, y, z, w;
	
	inline Vec4(): x(0), y(0), z(0), w(0) {}
	inline Vec4(const Vec3<T>& xyz, T w): x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}
	inline Vec4(T x, T y, T z, T w): x(x), y(y), z(z), w(w) {}
};

using Vec4i = Vec4<int>;
using Vec4f = Vec4<float>;
using Vec4d = Vec4<double>;
