#pragma once

#include "Vec3.hpp"

template <typename T>
struct Vec4
{
	T x, y, z, w;
	
	inline Vec4(): x(0), y(0), z(0), w(0) {}
	inline Vec4(const Vec3<T>& xyz, T w): x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}
	inline Vec4(T x, T y, T z, T w): x(x), y(y), z(z), w(w) {}

	inline T* ValuePointer() { return &x; }
	inline const T* ValuePointer() const { return &x; }

	inline T& operator[](std::size_t index) { return (&x)[index]; }
	inline const T& operator[](std::size_t index) const { return (&x)[index]; }

	inline Vec3<T> xyz() const { return Vec3<T>(x, y, z); }
};

using Vec4i = Vec4<int>;
using Vec4f = Vec4<float>;
using Vec4d = Vec4<double>;
