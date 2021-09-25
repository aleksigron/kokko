#pragma once

#include <cmath>
#include <cstddef>

template <typename T>
struct Vec2
{
	T x, y;
	
	Vec2(): x(0), y(0) {}
	Vec2(T x, T y): x(x), y(y) {}

	T* ValuePointer() { return &x; }
	const T* ValuePointer() const { return &x; }

	T& operator[](size_t index) { return (&x)[index]; }
	const T& operator[](size_t index) const { return (&x)[index]; }

	template <typename CastType>
	Vec2<CastType> As() const
	{
		return Vec2<CastType>(static_cast<CastType>(x), static_cast<CastType>(y));
	}
	
	// Normalize the vector
	void Normalize()
	{
		T magnitude = this->Magnitude();
		x /= magnitude;
		y /= magnitude;
	}
	
	// Return a normalized version of the vector
	Vec2 GetNormalized() const
	{
		Vec2<T> normalized = *this;
		normalized.Normalize();
		return normalized;
	}
	
	// Magnitude of the vector
	T Magnitude() const
	{
		return T(std::sqrt((x * x) + (y * y)));
	}
	
	// Squared magnitude of the vector
	T SqrMagnitude() const
	{
		return (x * x) + (y * y);
	}
	
	// Negation of the vector
	Vec2 operator-() const
	{
		return Vec2(-x, -y);
	}

	// In-place addition
	Vec2& operator+=(const Vec2& value)
	{
		x += value.x;
		y += value.y;

		return *this;
	}

	// In-place subtraction
	Vec2& operator-=(const Vec2& value)
	{
		x -= value.x;
		y -= value.y;

		return *this;
	}

	bool operator==(const Vec2& other)
	{
		return x == other.x && y == other.y;
	}

	bool operator!=(const Vec2& other)
	{
		return operator==(other) == false;
	}

	// Component-wise multiplication (Hadamard product) of two vectors
	static Vec2 Hadamard(const Vec2& lhs, const Vec2& rhs)
	{
		return Vec2(lhs.x * rhs.x, lhs.y * rhs.y);
	}
	
	// Dot product of two vectors
	static T Dot(const Vec2& lhs, const Vec2& rhs)
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
