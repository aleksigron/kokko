#pragma once

#include "Math/Vec2.hpp"

struct Mat2x2f
{
	float m[4];

	inline Mat2x2f(): m{ 1, 0, 0, 1 } {}

	inline float& operator[](std::size_t index) { return m[index]; }
	inline const float& operator[](std::size_t index) const { return m[index]; }

	inline void Transpose()
	{
		float temp = m[1];
		m[1] = m[2];
		m[2] = temp;
	}

	inline Mat2x2f GetTransposed() const
	{
		Mat2x2f result;

		result[0] = m[0];
		result[1] = m[2];
		result[2] = m[1];
		result[3] = m[3];

		return result;
	}

	inline float* ValuePointer() { return m; }
};

inline Vec2f operator*(const Mat2x2f& m, const Vec2f& v)
{
	return Vec2f(m[0] * v.x + m[2] * v.y,
				 m[1] * v.x + m[3] * v.y);
}

inline Vec2f operator*(const Vec2f& v, const Mat2x2f& m)
{
	return Vec2f(m[0] * v.x + m[1] * v.x,
				 m[2] * v.y + m[3] * v.y);
}

inline Mat2x2f operator*(const Mat2x2f& a, const Mat2x2f& b)
{
	Mat2x2f result;

	result[0] = a[0] * b[0] + a[2] * b[1];
	result[1] = a[1] * b[0] + a[3] * b[1];

	result[2] = a[0] * b[2] + a[2] * b[3];
	result[3] = a[1] * b[2] + a[3] * b[3];

	return result;
}
