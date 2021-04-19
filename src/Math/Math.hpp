#pragma once

#include <algorithm>
#include <limits>

#include "Math/Vec3.hpp"

namespace Math
{
	namespace Const
	{
		constexpr float HalfPi = 1.5707963267948966192313f;
		constexpr float Pi = 3.1415926535897932384626f;
		constexpr float	Tau = 6.2831853071795864769252f;
		
		constexpr float DegToRad = Tau / 360.0f;
		constexpr float RadToDeg = 360.0f / Tau;
		
		constexpr float Sqrt2 = 1.4142135623730950488016f;
		
		constexpr float e = 2.7182818284590452353602f;
		
		constexpr float GoldenRatio = 1.6180339887498948482045f;
	}
	
	constexpr float DegreesToRadians(float degrees)
	{
		return degrees * Const::DegToRad;
	}

	inline Vec3f DegreesToRadians(const Vec3f& degrees)
	{
		return degrees * Const::DegToRad;
	}
	
	constexpr float RadiansToDegrees(float radians)
	{
		return radians * Const::RadToDeg;
	}

	inline Vec3f RadiansToDegrees(const Vec3f& degrees)
	{
		return degrees * Const::RadToDeg;
	}

	inline float Lerp(float a, float b, float t)
	{
		// Based on https://github.com/emsr/cxx_linear/blob/master/lerp.h

		if (std::isnan(a) || std::isnan(b) || std::isnan(t))
			return std::numeric_limits<float>::quiet_NaN();
		else if ((a <= 0.0f && b >= 0.0f) || (a >= 0.0f && b <= 0.0f))
			return t * b + (1.0f - t) * a;
		else if (t == 1.0f)
			return b;
		else
		{
			const float x = a + t * (b - a);
			return (t > 1.0f) == (b > a) ? std::max(b, x) : std::min(b, x);
		}
	}

	constexpr unsigned int UpperPowerOfTwo(unsigned int v)
	{
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v++;
		return v;
	}

	constexpr unsigned long long UpperPowerOfTwo(unsigned long long v)
	{
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v |= v >> 32;
		v++;
		return v;
	}
}
