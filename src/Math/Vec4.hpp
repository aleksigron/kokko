#pragma once

#include "Math/Vec3.hpp"

template <typename T>
struct Vec4
{
	T x, y, z, w;
	
	Vec4(): x(0), y(0), z(0), w(0) {}
	Vec4(const Vec3<T>& xyz, T w): x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}
	Vec4(T x, T y, T z, T w): x(x), y(y), z(z), w(w) {}

	T* ValuePointer() { return &x; }
	const T* ValuePointer() const { return &x; }

	T& operator[](std::size_t index) { return (&x)[index]; }
	const T& operator[](std::size_t index) const { return (&x)[index]; }

	Vec3<T> xyz() const { return Vec3<T>(x, y, z); }

	// Component-wise multiplication (Hadamard product) of two vectors
	static inline Vec4 Hadamard(const Vec4& lhs, const Vec4& rhs)
	{
		return Vec4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w);
	}
};

// Vector-vector addition
template <typename T>
inline Vec4<T> operator+(const Vec4<T>& lhs, const Vec4<T>& rhs)
{
	return Vec4<T>(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w);
}

// Vector-vector substraction
template <typename T>
inline Vec4<T> operator-(const Vec4<T>& lhs, const Vec4<T>& rhs)
{
	return Vec4<T>(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w);
}

// Vector-scalar multiplication
template <typename T>
inline Vec4<T> operator*(const Vec4<T>& lhs, const T& rhs)
{
	return Vec4<T>(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs);
}

// Scalar-vector multiplication
template <typename T>
inline Vec4<T> operator*(const T& lhs, const Vec4<T>& rhs)
{
	return Vec4<T>(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w);
}

using Vec4i = Vec4<int>;
using Vec4f = Vec4<float>;
using Vec4d = Vec4<double>;
