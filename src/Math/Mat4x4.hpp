#pragma once

#include "Math/Vec4.hpp"
#include "Math/Mat3x3.hpp"

struct alignas(16) Mat4x4f
{
	float m[16];

	Mat4x4f():
	m { 1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1 }
	{
	}

	Mat4x4f(const Mat3x3f& m3):
	m { m3[0], m3[1], m3[2], 0,
		m3[3], m3[4], m3[5], 0,
		m3[6], m3[7], m3[8], 0,
		0,     0,     0,     1 }
	{
	}
	
	float& operator[](std::size_t index) { return m[index]; }
	const float& operator[](std::size_t index) const { return m[index]; }
	
	float* ValuePointer() { return m; }
	const float* ValuePointer() const { return m; }

	Mat3x3f Get3x3() const
	{
		Mat3x3f result;

		result[0] = m[0];
		result[1] = m[1];
		result[2] = m[2];

		result[3] = m[4];
		result[4] = m[5];
		result[5] = m[6];

		result[6] = m[8];
		result[7] = m[9];
		result[8] = m[10];

		return result;
	}

	void Transpose()
	{
		float temp;
		unsigned int pri = 1, sec;

		do
		{
			if (pri % 4 == 0)
				pri += pri / 4 + 1;

			sec = (pri % 4) * 4 + pri / 4;
			temp = m[sec];
			m[sec] = m[pri];
			m[pri] = temp;

			++pri;
		}
		while (pri < 12);
	}

	Mat4x4f GetTransposed() const
	{
		Mat4x4f result;

		for (unsigned int i = 0; i < 16; ++i)
			result[i] = m[(i % 4) * 4 + i / 4];

		return result;
	}

	/*
	Get the inverse of a non-scaled transform
	*/
	Mat4x4f GetInverse()
	{
		Mat3x3f inverseRotation = Get3x3().GetTransposed();
		Vec3f translation = -(inverseRotation * Vec3f(m[12], m[13], m[14]));

		Mat4x4f inverse(inverseRotation);
		inverse[12] = translation.x;
		inverse[13] = translation.y;
		inverse[14] = translation.z;

		return inverse;
	}

	static Mat4x4f RotateAroundAxis(Vec3f axis, float angle)
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

		Mat4x4f result;

		result[0] = xx + (1.0f - xx) * ca;
		result[1] = xy * (1.0f - ca) + axis.z * sa;
		result[2] = xz * (1.0f - ca) - axis.y * sa;
		result[3] = 0.0f;

		result[4] = xy * (1.0f - ca) - axis.z * sa;
		result[5] = yy + (1.0f - yy) * ca;
		result[6] = yz * (1.0f - ca) + axis.x * sa;
		result[7] = 0.0f;

		result[8] = xz * (1.0f - ca) + axis.y * sa;
		result[9] = yz * (1.0f - ca) - axis.x * sa;
		result[10] = zz + (1.0f - zz) * ca;
		result[11] = 0.0f;

		result[12] = 0.0f;
		result[13] = 0.0f;
		result[14] = 0.0f;
		result[15] = 1.0f;

		return result;
	}

	static Mat4x4f LookAt(Vec3f eye, Vec3f target, Vec3f up)
	{
		Mat4x4f result;

		Vec3f zaxis = (eye - target).GetNormalized();
		Vec3f xaxis = (Vec3f::Cross(up, zaxis)).GetNormalized();
		Vec3f yaxis = Vec3f::Cross(zaxis, xaxis);

		result[0] = xaxis.x;
		result[1] = xaxis.y;
		result[2] = xaxis.z;
		result[3] = 0.0f;

		result[4] = yaxis.x;
		result[5] = yaxis.y;
		result[6] = yaxis.z;
		result[7] = 0.0f;

		result[8] = zaxis.x;
		result[9] = zaxis.y;
		result[10] = zaxis.z;
		result[11] = 0.0f;

		result[12] = eye.x;
		result[13] = eye.y;
		result[14] = eye.z;
		result[15] = 1.0f;

		return result;
	}

	static Mat4x4f Translate(const Vec3f& translation)
	{
		Mat4x4f result;

		result[12] = translation.x;
		result[13] = translation.y;
		result[14] = translation.z;

		return result;
	}

	static Mat4x4f RotateEuler(const Vec3f& angles)
	{
		return Mat4x4f(Mat3x3f::RotateEuler(angles));
	}

	static Mat4x4f Scale(const Vec3f& scale)
	{
		Mat4x4f result;

		result[0] = scale.x;
		result[5] = scale.y;
		result[10] = scale.z;

		return result;
	}

	static Mat4x4f Scale(float scale)
	{
		Mat4x4f result;

		result[0] = scale;
		result[5] = scale;
		result[10] = scale;

		return result;
	}

	static void MultiplyMany(unsigned int count, const Mat4x4f* a, const Mat4x4f* b, Mat4x4f* out);
	static void MultiplyOneByMany(const Mat4x4f& a, unsigned int count, const Mat4x4f* b, Mat4x4f* out);
};

inline Vec4f operator*(const Mat4x4f& m, const Vec4f& v)
{
	return Vec4f(m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12] * v.w,
				 m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13] * v.w,
				 m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14] * v.w,
				 m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15] * v.w);
}

inline Vec4f operator*(const Vec4f& v, const Mat4x4f& m)
{
	return Vec4f(m[0] * v.x + m[1] * v.x + m[2] * v.x + m[3] * v.x,
				 m[4] * v.y + m[5] * v.y + m[6] * v.y + m[7] * v.y,
				 m[8] * v.z + m[9] * v.z + m[10] * v.z + m[11] * v.z,
				 m[12] * v.w + m[13] * v.w + m[14] * v.w + m[15] * v.w);
}

inline Mat4x4f operator*(const Mat4x4f& a, const Mat4x4f& b)
{
	Mat4x4f result;

	result[0] = a[0] * b[0] + a[4] * b[1] + a[8] * b[2] + a[12] * b[3];
	result[1] = a[1] * b[0] + a[5] * b[1] + a[9] * b[2] + a[13] * b[3];
	result[2] = a[2] * b[0] + a[6] * b[1] + a[10] * b[2] + a[14] * b[3];
	result[3] = a[3] * b[0] + a[7] * b[1] + a[11] * b[2] + a[15] * b[3];

	result[4] = a[0] * b[4] + a[4] * b[5] + a[8] * b[6] + a[12] * b[7];
	result[5] = a[1] * b[4] + a[5] * b[5] + a[9] * b[6] + a[13] * b[7];
	result[6] = a[2] * b[4] + a[6] * b[5] + a[10] * b[6] + a[14] * b[7];
	result[7] = a[3] * b[4] + a[7] * b[5] + a[11] * b[6] + a[15] * b[7];

	result[8] = a[0] * b[8] + a[4] * b[9] + a[8] * b[10] + a[12] * b[11];
	result[9] = a[1] * b[8] + a[5] * b[9] + a[9] * b[10] + a[13] * b[11];
	result[10] = a[2] * b[8] + a[6] * b[9] + a[10] * b[10] + a[14] * b[11];
	result[11] = a[3] * b[8] + a[7] * b[9] + a[11] * b[10] + a[15] * b[11];

	result[12] = a[0] * b[12] + a[4] * b[13] + a[8] * b[14] + a[12] * b[15];
	result[13] = a[1] * b[12] + a[5] * b[13] + a[9] * b[14] + a[13] * b[15];
	result[14] = a[2] * b[12] + a[6] * b[13] + a[10] * b[14] + a[14] * b[15];
	result[15] = a[3] * b[12] + a[7] * b[13] + a[11] * b[14] + a[15] * b[15];

	return result;
}
