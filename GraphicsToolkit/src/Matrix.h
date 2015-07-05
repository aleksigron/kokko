#pragma once

#include <cmath>

#include "Mat4x4.h"
#include "Vec3.h"

namespace Matrix
{
	template <typename T>
	Mat4x4<T> Perspective(T fovVertical, T aspectRatio, T near, T far)
	{
		T const tanHalfFovy = std::tan(fovVertical / static_cast<T>(2));
		
		Mat4x4<T> result;
		
		result[0] = static_cast<T>(1) / (aspectRatio * tanHalfFovy);
		result[5] = static_cast<T>(1) / (tanHalfFovy);
		result[10] = - (far + near) / (far - near);
		result[11] = - static_cast<T>(1);
		result[14] = - (static_cast<T>(2) * far * near) / (far - near);
		
		return result;
	}
	
	template <typename T>
	Mat4x4<T> Orthographic(T left, T right, T bottom, T top, T near, T far)
	{
		Mat4x4<T> result(Mat4x4<T>::identity);
		
		result[0] = static_cast<T>(2) / (right - left);
		result[5] = static_cast<T>(2) / (top - bottom);
		result[10] = - static_cast<T>(2) / (far - near);
		result[12] = - (right + left) / (right - left);
		result[13] = - (top + bottom) / (top - bottom);
		result[14] = - (far + near) / (far - near);
		
		return result;
	}
	
	template <typename T>
	Mat4x4<T> Orthographic(T halfWidth, T halfHeight, T near, T far)
	{
		Mat4x4<T> result(Mat4x4<T>::identity);
		
		result[0] = static_cast<T>(1) / halfWidth;
		result[5] = static_cast<T>(1) / halfHeight;
		result[10] = static_cast<T>(-2) / (far - near);
		result[14] = - (far + near) / (far - near);
		
		return result;
	}
	
	template <typename T>
	inline Mat4x4<T> Translate(const Vec3<T>& translation)
	{
		Mat4x4<T> result(Mat4x4<T>::identity);
		
		result[12] = translation.x;
		result[13] = translation.y;
		result[14] = translation.z;
		
		return result;
	}
	
	template <typename T>
	inline Mat4x4<T> Scale(const Vec3<T> scaling)
	{
		Mat4x4<T> result(Mat4x4<T>::identity);
		
		result[0] = scaling.x;
		result[5] = scaling.y;
		result[10] = scaling.z;
		
		return result;
	}
	
	template <typename T>
	inline Mat4x4<T> Rotate(Vec3<T> axis, T angle)
	{
		axis.Normalize();
		
		const T xx = axis.x * axis.x;
		const T yy = axis.y * axis.y;
		const T zz = axis.z * axis.z;
		
		const T xy = axis.x * axis.y;
		const T xz = axis.x * axis.z;
		const T yz = axis.y * axis.z;
		
		const T ca = static_cast<T>(std::cos(angle));
		const T sa = static_cast<T>(std::sin(angle));
		
		Mat4x4<T> result;
		
		result[0] = xx + (static_cast<T>(1) - xx) * ca;
		result[1] = xy * (static_cast<T>(1) - ca) + axis.z * sa;
		result[2] = xz * (static_cast<T>(1) - ca) - axis.y * sa;
		
		result[4] = xy * (static_cast<T>(1) - ca) - axis.z * sa;
		result[5] = yy + (static_cast<T>(1) - yy) * ca;
		result[6] = yz * (static_cast<T>(1) - ca) + axis.x * sa;
		
		result[8] = xz * (static_cast<T>(1) - ca) + axis.y * sa;
		result[9] = yz * (static_cast<T>(1) - ca) - axis.x * sa;
		result[10] = zz + (static_cast<T>(1) - zz) * ca;
		
		result[15] = static_cast<T>(1);
		
		return result;
	}
}