#pragma once

#include "Vec3.h"

struct Mat3x3f
{
	struct Uninitialize {} static uninit;

	float m[9];

	inline Mat3x3f(Uninitialize) {}
	inline Mat3x3f(): m{ 1, 0, 0, 0, 1, 0, 0, 0, 1 } {}

	inline float& operator[](std::size_t index) { return m[index]; }
	inline const float& operator[](std::size_t index) const { return m[index]; }

	inline float* ValuePointer() { return m; }
};

inline Vec3f operator*(const Mat3x3f& m, const Vec3f& v)
{
	return Vec3f(m[0] * v.x + m[3] * v.y + m[6] * v.z,
				 m[1] * v.x + m[4] * v.y + m[7] * v.z,
				 m[2] * v.x + m[5] * v.y + m[8] * v.z);
}

inline Vec3f operator*(const Vec3f& v, const Mat3x3f& m)
{
	return Vec3f(m[0] * v.x + m[1] * v.x + m[2] * v.x,
				 m[3] * v.y + m[4] * v.y + m[5] * v.y,
				 m[6] * v.z + m[7] * v.z + m[8] * v.z);
}

inline Mat3x3f operator*(const Mat3x3f& a, const Mat3x3f& b)
{
	Mat3x3f result(uninit);

	result[0] = a[0] * b[0] + a[3] * b[1] + a[6] * b[2];
	result[1] = a[1] * b[0] + a[4] * b[1] + a[7] * b[2];
	result[2] = a[2] * b[0] + a[5] * b[1] + a[8] * b[2];

	result[3] = a[0] * b[3] + a[3] * b[4] + a[6] * b[5];
	result[4] = a[1] * b[3] + a[4] * b[4] + a[7] * b[5];
	result[5] = a[2] * b[3] + a[5] * b[4] + a[8] * b[5];

	result[6] = a[0] * b[6] + a[3] * b[7] + a[6] * b[8];
	result[7] = a[1] * b[6] + a[4] * b[7] + a[7] * b[8];
	result[8] = a[2] * b[6] + a[5] * b[7] + a[8] * b[8];

	return result;
}
