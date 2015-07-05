#pragma once

#include "Vec3.h"
#include "Vec4.h"

template <typename T>
struct alignas(sizeof(T) * 16) Mat4x4
{
	struct ZeroInit {} static zero;
	struct IdentityInit {} static identity;
	
	T m[16];
	
	inline Mat4x4() {}
	inline Mat4x4(ZeroInit): m{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } {}
	inline Mat4x4(IdentityInit): m{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 } {}
	inline Mat4x4(T fill):
		m { fill, fill, fill, fill, fill, fill, fill, fill,
			fill, fill, fill, fill, fill, fill, fill, fill }
	{
	}
	
	inline T& operator[](std::size_t index) { return m[index]; }
	inline const T& operator[](std::size_t index) const { return m[index]; }
	
	inline T* ValuePointer() { return &m[0]; }
};

template <typename T>
inline Vec4<T> operator*(const Mat4x4<T>& m, const Vec4<T>& v)
{
	return Vec4<T>(m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12] * v.w,
				   m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13] * v.w,
				   m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14] * v.w,
				   m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15] * v.w);
}

template <typename T>
inline Vec4<T> operator*(const Vec4<T>& v, const Mat4x4<T>& m)
{
	return Vec4<T>(m[0] * v.x + m[1] * v.x + m[2] * v.x + m[3] * v.x,
				   m[4] * v.y + m[5] * v.y + m[6] * v.y + m[7] * v.y,
				   m[8] * v.z + m[9] * v.z + m[10] * v.z + m[11] * v.z,
				   m[12] * v.w + m[13] * v.w + m[14] * v.w + m[15] * v.w);
}

template <typename T>
Mat4x4<T> operator*(const Mat4x4<T>& a, const Mat4x4<T>& b)
{
	Mat4x4<T> result;
	
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

using Mat4x4f = Mat4x4<float>;
using Mat4x4d = Mat4x4<double>;