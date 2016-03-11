#pragma once

#include "Vec3.hpp"

struct Mat3x3f
{
	struct Uninitialize {} static uninit;

	float m[9];

	inline Mat3x3f(Uninitialize) {}
	inline Mat3x3f(): m{ 1, 0, 0, 0, 1, 0, 0, 0, 1 } {}

	inline float& operator[](std::size_t index) { return m[index]; }
	inline const float& operator[](std::size_t index) const { return m[index]; }

	inline float* ValuePointer() { return m; }

	inline void Transpose()
	{
		float temp = m[1];
		m[1] = m[3];
		m[3] = temp;

		temp = m[2];
		m[2] = m[6];
		m[6] = temp;

		temp = m[5];
		m[5] = m[8];
		m[8] = temp;
	}

	inline Mat3x3f GetTransposed() const
	{
		Mat3x3f result(Mat3x3f::uninit);

		for (unsigned int i = 0; i < 9; ++i)
			result[i] = m[(i % 3) * 3 + i / 3];

		return result;
	}

	static inline Mat3x3f RotateAroundAxis(Vec3f axis, float angle)
	{
		axis.Normalize();

		const float xx = axis.x * axis.x;
		const float yy = axis.y * axis.y;
		const float zz = axis.z * axis.z;

		const float xy = axis.x * axis.y;
		const float xz = axis.x * axis.z;
		const float yz = axis.y * axis.z;

		const float ca = std::cosf(angle);
		const float sa = std::sinf(angle);

		Mat3x3f result(uninit);

		result[0] = xx + (1.0f - xx) * ca;
		result[1] = xy * (1.0f - ca) + axis.z * sa;
		result[2] = xz * (1.0f - ca) - axis.y * sa;

		result[3] = xy * (1.0f - ca) - axis.z * sa;
		result[4] = yy + (1.0f - yy) * ca;
		result[5] = yz * (1.0f - ca) + axis.x * sa;

		result[6] = xz * (1.0f - ca) + axis.y * sa;
		result[7] = yz * (1.0f - ca) - axis.x * sa;
		result[8] = zz + (1.0f - zz) * ca;

		return result;
	}
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
	Mat3x3f result(Mat3x3f::uninit);

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
