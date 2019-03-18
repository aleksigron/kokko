#pragma once

#include "Vec3.hpp"

struct Mat3x3f
{
	float m[9];


	Mat3x3f(): m{ 1, 0, 0, 0, 1, 0, 0, 0, 1 } {}

	float& operator[](unsigned int index) { return m[index]; }
	const float& operator[](unsigned int index) const { return m[index]; }

	float* ValuePointer() { return m; }

	Vec3f Right() const { return Vec3f(m[0], m[1], m[2]); }
	Vec3f Up() const { return Vec3f(m[3], m[4], m[5]); }
	Vec3f Forward() const { return Vec3f(-m[6], -m[7], -m[8]); }

	Mat3x3f operator*(const Mat3x3f& b)
	{
		Mat3x3f result;

		result[0] = m[0] * b[0] + m[3] * b[1] + m[6] * b[2];
		result[1] = m[1] * b[0] + m[4] * b[1] + m[7] * b[2];
		result[2] = m[2] * b[0] + m[5] * b[1] + m[8] * b[2];

		result[3] = m[0] * b[3] + m[3] * b[4] + m[6] * b[5];
		result[4] = m[1] * b[3] + m[4] * b[4] + m[7] * b[5];
		result[5] = m[2] * b[3] + m[5] * b[4] + m[8] * b[5];

		result[6] = m[0] * b[6] + m[3] * b[7] + m[6] * b[8];
		result[7] = m[1] * b[6] + m[4] * b[7] + m[7] * b[8];
		result[8] = m[2] * b[6] + m[5] * b[7] + m[8] * b[8];

		return result;
	}

	void Transpose()
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

	Mat3x3f GetTransposed() const
	{
		Mat3x3f result;

		for (unsigned int i = 0; i < 9; ++i)
			result[i] = m[(i % 3) * 3 + i / 3];

		return result;
	}

	static Mat3x3f RotateEuler(const Vec3f& angles)
	{
		Mat3x3f x, y, z;

		x[0] = 1.0f;
		x[1] = 0.0f;
		x[2] = 0.0f;
		x[3] = 0.0f;
		x[4] = std::cosf(angles.x);
		x[5] = std::sinf(angles.x);
		x[6] = 0.0f;
		x[7] = -std::sinf(angles.x);
		x[8] = std::cosf(angles.x);

		y[0] = std::cosf(angles.y);
		y[1] = 0.0f;
		y[2] = -std::sinf(angles.y);
		y[3] = 0.0f;
		y[4] = 1.0f;
		y[5] = 0.0f;
		y[6] = std::sinf(angles.y);
		y[7] = 0.0f;
		y[8] = std::cosf(angles.y);

		z[0] = std::cosf(angles.z);
		z[1] = -std::sinf(angles.z);
		z[2] = 0.0f;
		z[3] = std::sinf(angles.z);
		z[4] = std::cosf(angles.z);
		z[5] = 0.0f;
		z[6] = 0.0f;
		z[7] = 0.0f;
		z[8] = 1.0f;

		return z * y * x;
	}

	static Mat3x3f RotateAroundAxis(Vec3f axis, float angle)
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

		Mat3x3f result;

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
