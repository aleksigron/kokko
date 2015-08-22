#pragma once

#include <cmath>

template <typename T>
struct Vec2
{
	T x, y;
	
	inline Vec2(): x(0), y(0) {}
	inline Vec2(T x, T y): x(x), y(y) {}
	
	// Normalize the vector
	inline void Normalize()
	{
		T magnitude = this->Magnitude();
		x /= magnitude;
		y /= magnitude;
	}
	
	// Return a normalized version of the vector
	inline Vec2 Normalized() const
	{
		Vec2<T> normalized = *this;
		normalized.Normalize();
		return normalized;
	}
	
	// Magnitude of the vector
	inline T Magnitude() const
	{
		return T(std::sqrt((x * x) + (y * y)));
	}
	
	// Squared magnitude of the vector
	inline T SqrMagnitude() const
	{
		return (x * x) + (y * y);
	}
	
	// Negation of the vector
	inline Vec2 operator-() const
	{
		return Vec2(-x, -y);
	}

	// In-place addition
	inline Vec2& operator+=(const Vec2& value)
	{
		x += value.x;
		y += value.y;

		return *this;
	}

	// In-place subtraction
	inline Vec2& operator-=(const Vec2& value)
	{
		x -= value.x;
		y -= value.y;

		return *this;
	}
	
	// Dot product of two vectors
	static inline T Dot(const Vec2& lhs, const Vec2& rhs)
	{
		return (lhs.x * rhs.x) + (lhs.y * rhs.y);
	}
};

// Vector-vector addition
template <typename T>
inline Vec2<T> operator+(const Vec2<T>& lhs, const Vec2<T>& rhs)
{
	return Vec2<T>(lhs.x + rhs.x, lhs.y + rhs.y);
}

// Vector-vector substraction
template <typename T>
inline Vec2<T> operator-(const Vec2<T>& lhs, const Vec2<T>& rhs)
{
	return Vec2<T>(lhs.x - rhs.x, lhs.y - rhs.y);
}

// Vector-scalar multiplication
template <typename T>
inline Vec2<T> operator*(const Vec2<T>& lhs, const T& rhs)
{
	return Vec2<T>(lhs.x * rhs, lhs.y * rhs);
}

// Scalar-vector multiplication
template <typename T>
inline Vec2<T> operator*(const T& lhs, const Vec2<T>& rhs)
{
	return Vec2<T>(lhs * rhs.x, lhs * rhs.y);
}

using Vec2i = Vec2<int>;
using Vec2f = Vec2<float>;
using Vec2d = Vec2<double>;
