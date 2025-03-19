#pragma once

#include "Math/Vec3.hpp"

namespace kokko
{

struct Color
{
	static Vec3f FromSrgbToLinear(const Vec3f& srgb);
	static Vec3f FromLinearToSrgb(const Vec3f& srgb);

	float r, g, b, a;

	Color()
	{
	}

	Color(float r, float g, float b) : r(r), g(g), b(b), a(1.0f)
	{
	}

	Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a)
	{
	}

	float* ValuePointer() { return &r; }
	const float* ValuePointer() const { return &r; }

	Color& operator*=(const Color& value)
	{
		this->r *= value.r;
		this->g *= value.g;
		this->b *= value.b;
		this->a *= value.a;

		return *this;
	}

	Color& operator*=(float value)
	{
		this->r *= value;
		this->g *= value;
		this->b *= value;
		this->a *= value;

		return *this;
	}

	Color& operator+=(const Color& value)
	{
		this->r += value.r;
		this->g += value.g;
		this->b += value.b;
		this->a += value.a;

		return *this;
	}

	Color& operator-=(const Color& value)
	{
		this->r -= value.r;
		this->g -= value.g;
		this->b -= value.b;
		this->a -= value.a;

		return *this;
	}
};

inline Color operator*(const Color& lhs, float rhs)
{
	return Color(lhs.r * rhs, lhs.g * rhs, lhs.b * rhs, lhs.a * rhs);
}

inline Color operator*(float lhs, const Color& rhs)
{
	return operator*(rhs, lhs);
}

inline Color operator*(const Color& lhs, const Color& rhs)
{
	return Color(lhs.r * rhs.r, lhs.g * rhs.g, lhs.b * rhs.b, lhs.a * rhs.a);
}

inline Color operator+(const Color& lhs, const Color& rhs)
{
	return Color(lhs.r + rhs.r, lhs.g + rhs.g, lhs.b + rhs.b, lhs.a + rhs.a);
}

inline Color operator-(const Color& lhs, const Color& rhs)
{
	return Color(lhs.r - rhs.r, lhs.g - rhs.g, lhs.b - rhs.b, lhs.a - rhs.a);
}

} // namespace kokko
