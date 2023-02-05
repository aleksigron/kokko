#include "Core/Color.hpp"

#include <cassert>
#include <cmath>

#include "doctest/doctest.h"

Vec3f Color::FromSrgbToLinear(const Vec3f& srgb)
{
	auto toLinear = [](float v) { return v <= 0.04045f ? v / 12.92f : std::pow((v + 0.055f) / 1.055f, 2.4f); };
	return Vec3f(toLinear(srgb.x), toLinear(srgb.y), toLinear(srgb.z));
}

Vec3f Color::FromLinearToSrgb(const Vec3f& srgb)
{
	auto toSrgb = [](float v) { return v <= 0.0031308f ? v * 12.92f : 1.055f * std::pow(v, 1.0f / 2.4f) - 0.055f; };
	return Vec3f(toSrgb(srgb.x), toSrgb(srgb.y), toSrgb(srgb.z));
}

TEST_CASE("Color.ConvertLinearSrgb")
{
	Vec3f linearColors[] = {
		Vec3f(1.0f, 1.0f, 1.0f),
		Vec3f(0.5f, 0.5f, 0.5),
		Vec3f(0.0f, 0.0f, 0.0f),
		Vec3f(0.9f, 0.1f, 0.0f),
		Vec3f(0.0f, 0.9f, 0.1f),
		Vec3f(0.1f, 0.0f, 0.9f)
	};

	Vec3f srgbColors[] = {
		Vec3f(1.0f, 1.0f, 1.0f),
		Vec3f(0.73536f, 0.73536f, 0.73536f),
		Vec3f(0.0f, 0.0f, 0.0f),
		Vec3f(0.95469f, 0.34919f, 0.0f),
		Vec3f(0.0f, 0.95469f, 0.34919f),
		Vec3f(0.34919f, 0.0f, 0.95469f)
	};

	assert(sizeof(linearColors) == sizeof(srgbColors));

	for (size_t i = 0, count = sizeof(linearColors) / sizeof(linearColors[0]); i < count; ++i)
	{
		Vec3f linToSrgb = Color::FromLinearToSrgb(linearColors[i]);
		CHECK(linToSrgb[0] == doctest::Approx(srgbColors[i][0]));
		CHECK(linToSrgb[1] == doctest::Approx(srgbColors[i][1]));
		CHECK(linToSrgb[2] == doctest::Approx(srgbColors[i][2]));

		Vec3f linToSrgbToLin = Color::FromSrgbToLinear(linToSrgb);
		CHECK(linToSrgbToLin[0] == doctest::Approx(linearColors[i][0]));
		CHECK(linToSrgbToLin[1] == doctest::Approx(linearColors[i][1]));
		CHECK(linToSrgbToLin[2] == doctest::Approx(linearColors[i][2]));
	}
}
