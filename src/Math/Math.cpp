#include "Math/Math.hpp"

#include "doctest/doctest.h"

Vec3f Math::DegreesToRadians(const Vec3f& degrees)
{
	return degrees * Const::DegToRad;
}

Vec3f Math::RadiansToDegrees(const Vec3f& radians)
{
	return radians * Const::RadToDeg;
}

float Math::Lerp(float a, float b, float t)
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

bool Math::IsPowerOfTwo(unsigned int v)
{
	if (v == 0)
		return false;

	return (v & (v - 1)) == 0;
}

bool Math::IsPowerOfTwo(size_t v)
{
	if (v == 0)
		return false;

	return (v & (v - 1)) == 0;
}

TEST_CASE("Math::IsPowerOfTwo")
{
	CHECK(Math::IsPowerOfTwo(0u) == false);
	CHECK(Math::IsPowerOfTwo(1u) == true);
	CHECK(Math::IsPowerOfTwo(2u) == true);
	CHECK(Math::IsPowerOfTwo(3u) == false);
	CHECK(Math::IsPowerOfTwo(4u) == true);
	CHECK(Math::IsPowerOfTwo(5u) == false);
	CHECK(Math::IsPowerOfTwo(6u) == false);
	CHECK(Math::IsPowerOfTwo(7u) == false);
	CHECK(Math::IsPowerOfTwo(8u) == true);
	CHECK(Math::IsPowerOfTwo(9u) == false);

	CHECK(Math::IsPowerOfTwo(0ull) == false);
	CHECK(Math::IsPowerOfTwo(1ull) == true);
	CHECK(Math::IsPowerOfTwo(2ull) == true);
	CHECK(Math::IsPowerOfTwo(3ull) == false);
	CHECK(Math::IsPowerOfTwo(4ull) == true);
	CHECK(Math::IsPowerOfTwo(5ull) == false);
	CHECK(Math::IsPowerOfTwo(6ull) == false);
	CHECK(Math::IsPowerOfTwo(7ull) == false);
	CHECK(Math::IsPowerOfTwo(8ull) == true);
	CHECK(Math::IsPowerOfTwo(9ull) == false);

	CHECK(Math::IsPowerOfTwo((1ull << 32) - 1) == false);
	CHECK(Math::IsPowerOfTwo((1ull << 32) + 0) == true);
	CHECK(Math::IsPowerOfTwo((1ull << 32) + 1) == false);
}

TEST_CASE("Math::UpperPowerOfTwo")
{
	CHECK(Math::UpperPowerOfTwo(0u) == 0u);
	CHECK(Math::UpperPowerOfTwo(1u) == 1u);
	CHECK(Math::UpperPowerOfTwo(2u) == 2u);
	CHECK(Math::UpperPowerOfTwo(3u) == 4u);
	CHECK(Math::UpperPowerOfTwo(4u) == 4u);
	CHECK(Math::UpperPowerOfTwo(5u) == 8u);
	CHECK(Math::UpperPowerOfTwo(6u) == 8u);
	CHECK(Math::UpperPowerOfTwo(7u) == 8u);
	CHECK(Math::UpperPowerOfTwo(8u) == 8u);
	CHECK(Math::UpperPowerOfTwo(9u) == 16u);

	CHECK(Math::UpperPowerOfTwo(0ull) == 0ull);
	CHECK(Math::UpperPowerOfTwo(1ull) == 1ull);
	CHECK(Math::UpperPowerOfTwo(2ull) == 2ull);
	CHECK(Math::UpperPowerOfTwo(3ull) == 4ull);
	CHECK(Math::UpperPowerOfTwo(4ull) == 4ull);
	CHECK(Math::UpperPowerOfTwo(5ull) == 8ull);
	CHECK(Math::UpperPowerOfTwo(6ull) == 8ull);
	CHECK(Math::UpperPowerOfTwo(7ull) == 8ull);
	CHECK(Math::UpperPowerOfTwo(8ull) == 8ull);
	CHECK(Math::UpperPowerOfTwo(9ull) == 16ull);

	CHECK(Math::UpperPowerOfTwo((1ull << 32) - 1) == 1ull << 32);
	CHECK(Math::UpperPowerOfTwo((1ull << 32) + 0) == 1ull << 32);
	CHECK(Math::UpperPowerOfTwo((1ull << 32) + 1) == 1ull << 33);
}
