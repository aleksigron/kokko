#pragma once

#include <cmath>

#include "Color.hpp"

struct ColorSrgb
{
	unsigned char r, g, b, a;

	ColorSrgb() : r(0), g(0), b(0), a(0)
	{
	}

	ColorSrgb(const ColorSrgb& other) : r(other.r), g(other.g), b(other.b), a(other.a)
	{
	}

	ColorSrgb(const Color& linear)
	{
		this->FromLinear(linear);
	}

	ColorSrgb(unsigned char r, unsigned char g, unsigned char b) :
		r(r), g(g), b(b), a(255)
	{
	}

	ColorSrgb(unsigned char r, unsigned char g, unsigned char b, unsigned char a) :
		r(r), g(g), b(b), a(a)
	{
	}

	Color ToLinear()
	{
		Color linear;
		linear.r = std::pow(r / 255.0f, 1.0f / 2.2f);
		linear.g = std::pow(g / 255.0f, 1.0f / 2.2f);
		linear.b = std::pow(b / 255.0f, 1.0f / 2.2f);
		linear.a = a / 255.0f;

		return linear;
	}

	void FromLinear(const Color& linear)
	{
		this->r = static_cast<unsigned char>(std::pow(linear.r, 2.2f) * 255.99998474f);
		this->g = static_cast<unsigned char>(std::pow(linear.g, 2.2f) * 255.99998474f);
		this->b = static_cast<unsigned char>(std::pow(linear.b, 2.2f) * 255.99998474f);
		this->a = static_cast<unsigned char>(linear.a * 255.0f);
	}
};
