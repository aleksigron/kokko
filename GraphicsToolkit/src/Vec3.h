#pragma once

#include "Vec4.h"

template <typename T>
struct alignas(sizeof(T) * 4) Vec3
{
	T x, y, z;
	
	inline Vec3(): x(0), y(0), z(0) {}
	inline Vec3(T x, T y, T z): x(x), y(y), z(z) {}
	inline Vec3(const Vec4<T>& v): x(v.x), y(v.y), z(v.z) {}
	
	// Normalize the vector
	inline void Normalize()
	{
		T magnitude = this->Magnitude();
		x /= magnitude;
		y /= magnitude;
		z /= magnitude;
	}
	
	// Return a normalized version of the vector
	inline Vec3 Normalized() const
	{
		Vec3 normalized = *this;
		normalized.Normalize();
		return normalized;
	}
	
	// Magnitude of the vector
	inline T Magnitude() const
	{
		return T(sqrt((x * x) + (y * y) + (z * z)));
	}
	
	// Squared magnitude of the vector
	inline T SqrMagnitude() const
	{
		return (x * x) + (y * y) + (z * z);
	}
	
	// Negation of the vector
	inline Vec3 operator-() const
	{
		return Vec3(-x, -y, -z);
	}
	// A Vec4 of the this vector as a position
	Vec4<T> AsPosition() const
	{
		return Vec4<T>(x, y, z, static_cast<T>(0));
	}
	
	// A Vec4 of the this vector as a direction
	Vec4<T> AsDirection() const
	{
		return Vec4<T>(x, y, z, static_cast<T>(0));
	}
	
	// Dot product of two vectors
	static inline T Dot(const Vec3& lhs, const Vec3& rhs)
	{
		return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z);
	}
	
	// Dot product of two vectors
	static inline Vec3 Cross(const Vec3& lhs, const Vec3& rhs)
	{
		return Vec3(lhs.y * rhs.z - lhs.z * rhs.y,
					lhs.z * rhs.x - lhs.x * rhs.z,
					lhs.x * rhs.y - lhs.y * rhs.x);
	}
};

// Vector-vector addition
template <typename T>
inline Vec3<T> operator+(const Vec3<T>& lhs, const Vec3<T>& rhs)
{
	return Vec3<T>(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}

// Vector-vector substraction
template <typename T>
inline Vec3<T> operator-(const Vec3<T>& lhs, const Vec3<T>& rhs)
{
	return Vec3<T>(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

// Vector-scalar multiplication
template <typename T>
inline Vec3<T> operator*(const Vec3<T>& lhs, const T& rhs)
{
	return Vec3<T>(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs);
}

// Scalar-vector multiplication
template <typename T>
inline Vec3<T> operator*(const T& lhs, const Vec3<T>& rhs)
{
	return Vec3<T>(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
}

using Vec3i = Vec3<int>;
using Vec3f = Vec3<float>;
using Vec3d = Vec3<double>;