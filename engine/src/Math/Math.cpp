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

float Math::DampenMultiplier(float dampenPerSecond, float deltaTime)
{
	return std::pow(dampenPerSecond, deltaTime);
}

TEST_CASE("Math::DampenMultiplier")
{
	const float dampenPerSecond = 0.5f;
	const float startSpeed = 1.0f;
	const float singleResult = startSpeed * dampenPerSecond;

	SUBCASE("Fixed frametime")
	{
		int minFps = 20, maxFps = 400, fpsStep = 20;
		for (int fps = minFps; fps <= maxFps; ++fps)
		{
			float deltaTime = 1.0f / fps;
			float iterativeResult = startSpeed;
			for (int frame = 0; frame < fps; ++frame)
			{
				float dampen = Math::DampenMultiplier(dampenPerSecond, deltaTime);
				iterativeResult = iterativeResult * dampen;
			}

			CHECK(iterativeResult == doctest::Approx(singleResult));
		}
	}

	SUBCASE("Variable frametime")
	{
		float totalTime = 0.0f;
		float iterativeResult = startSpeed;

		for (int frame = 0; frame < 40; ++frame)
		{
			float deltaTime = ((frame % 4) + 1) * 0.01f;
			totalTime += deltaTime;
			float dampen = Math::DampenMultiplier(dampenPerSecond, deltaTime);
			iterativeResult = iterativeResult * dampen;
		}

		CHECK(iterativeResult == doctest::Approx(singleResult));
		CHECK(totalTime == doctest::Approx(1.0f));
	}
}

bool Math::IsPowerOfTwo(uint64_t v)
{
	if (v == 0)
		return false;

	return (v & (v - 1)) == 0;
}

TEST_CASE("Math::IsPowerOfTwo")
{
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
