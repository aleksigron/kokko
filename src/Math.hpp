#pragma

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
	
	constexpr float RadiansToDegrees(float radians)
	{
		return radians * Const::RadToDeg;
	}
}