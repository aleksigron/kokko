#pragma once

template <typename T>
class Vec3
{
public:
	T x;
	T y;
	T z;
	
	Vec3(): x(0), y(0), z(0) {}
	Vec3(T x, T y, T z): x(x), y(y), z(z) {}
};

using Vec3i = Vec3<int>;
using Vec3f = Vec3<float>;
using Vec3d = Vec3<double>;