#pragma once

#include <cmath>
#include <cstddef>

#include "Math/Vec3.hpp"

struct Mat3x3f
{
	float m[9];

	Mat3x3f();

	float& operator[](size_t index);
	const float& operator[](size_t index) const;

	float* ValuePointer();
	const float* ValuePointer() const;

	Vec3f Right() const;
	Vec3f Up() const;
	Vec3f Forward() const;

	void Transpose();
	Mat3x3f GetTransposed() const;

	static Mat3x3f RotateAroundAxis(Vec3f axis, float angle);
	static Mat3x3f RotateEuler(const Vec3f& angles);

	static Mat3x3f FromAxes(const Vec3f& x, const Vec3f& y, const Vec3f& z);
};

Mat3x3f operator*(const Mat3x3f& a, const Mat3x3f& b);
Vec3f operator*(const Mat3x3f& m, const Vec3f& v);
Vec3f operator*(const Vec3f& v, const Mat3x3f& m);
