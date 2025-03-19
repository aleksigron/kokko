#pragma once

#include <cmath>
#include <cstddef>

#include "Math/Vec4.hpp"

namespace kokko
{

struct Mat3x3f;

template <typename T>
struct Vec2;

template <typename T>
class Optional;

struct alignas(16) Mat4x4f
{
	float m[16];

	Mat4x4f();
	Mat4x4f(const Mat3x3f& m3);

	float& operator[](size_t index);
	const float& operator[](size_t index) const;

	float* ValuePointer();
	const float* ValuePointer() const;

	Mat3x3f Get3x3() const;

	void Transpose();
	Mat4x4f GetTransposed() const;

	Mat4x4f GetInverseNonScaled() const;
	Optional<Mat4x4f> GetInverse() const;

	static Mat4x4f LookAt(Vec3f eye, Vec3f target, Vec3f up);
	static Mat4x4f Translate(const Vec3f& translation);

	static Mat4x4f RotateAroundAxis(Vec3f axis, float angle);
	static Mat4x4f RotateEuler(const Vec3f& angles);

	static Mat4x4f Scale(const Vec3f& scale);
	static Mat4x4f Scale(float scale);

	static Mat4x4f ScreenSpaceProjection(const Vec2<int>& screenSize);

	static void MultiplyMany(unsigned int count, const Mat4x4f* a, const Mat4x4f* b, Mat4x4f* out);
	static void MultiplyOneByMany(const Mat4x4f& a, unsigned int count, const Mat4x4f* b, Mat4x4f* out);
};

Vec4f operator*(const Mat4x4f& m, const Vec4f& v);
Vec4f operator*(const Vec4f& v, const Mat4x4f& m);
Mat4x4f operator*(const Mat4x4f& a, const Mat4x4f& b);

struct Mat4x4fBijection
{
	Mat4x4f forward;
	Mat4x4f inverse;
};

} // namespace kokko
