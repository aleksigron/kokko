#include "Math/Mat3x3.hpp"

#include "doctest/doctest.h"

#include "Core/Optional.hpp"

namespace kokko
{

namespace
{

int Index(int row, int col)
{
	return row * 3 + col;
}

void MultiplyCells(Mat3x3f& m, float f)
{
	for (int i = 0; i < 9; ++i)
		m[i] = m[i] * f;
}

} // namespace

Mat3x3f::Mat3x3f() : m{ 1, 0, 0, 0, 1, 0, 0, 0, 1 } {}

float& Mat3x3f::operator[](size_t index) { return m[index]; }
const float& Mat3x3f::operator[](size_t index) const { return m[index]; }

float* Mat3x3f::ValuePointer() { return m; }
const float* Mat3x3f::ValuePointer() const { return m; }

float Mat3x3f::GetDeterminant() const
{
	float cofactors[3];

	for (int col = 0; col < 3; ++col)
	{
		int col0 = col > 0 ? 0 : 1;
		int row0 = 1;
		int col1 = col < 2 ? 2 : 1;
		int row1 = 2;

		float val00 = m[Index(row0, col0)];
		float val01 = m[Index(row0, col1)];
		float val10 = m[Index(row1, col0)];
		float val11 = m[Index(row1, col1)];

		float sign = (col & 1) != 0 ? -1.0f : 1.0f;
		cofactors[col] = (val00 * val11 - val01 * val10) * sign;
	}

	return m[0] * cofactors[0] - m[1] * cofactors[1] + m[2] * cofactors[2];
}

TEST_CASE("Mat3x3f.GetDeterminant")
{
	Mat3x3f matrix;
	matrix[0] = 3.0f;
	matrix[1] = 0.0f;
	matrix[2] = 2.0f;
	matrix[3] = 2.0f;
	matrix[4] = 0.0f;
	matrix[5] = -2.0f;
	matrix[6] = 0.0f;
	matrix[7] = 1.0f;
	matrix[8] = 1.0f;

	CHECK(matrix.GetDeterminant() == doctest::Approx(10.0f));
}

Vec3f Mat3x3f::Right() const
{
	return Vec3f(m[0], m[1], m[2]);
}

Vec3f Mat3x3f::Up() const
{
	return Vec3f(m[3], m[4], m[5]);
}

Vec3f Mat3x3f::Forward() const
{
	return Vec3f(-m[6], -m[7], -m[8]);
}

void Mat3x3f::Transpose()
{
	float temp = m[1];
	m[1] = m[3];
	m[3] = temp;

	temp = m[2];
	m[2] = m[6];
	m[6] = temp;

	temp = m[5];
	m[5] = m[8];
	m[8] = temp;
}

Mat3x3f Mat3x3f::GetTransposed() const
{
	Mat3x3f result;

	for (unsigned int i = 0; i < 9; ++i)
		result[i] = m[(i % 3) * 3 + i / 3];

	return result;
}

Optional<Mat3x3f> Mat3x3f::GetInverse() const
{
	Mat3x3f cofactors;

	for (int row = 0; row < 3; ++row)
	{
		for (int col = 0; col < 3; ++col)
		{
			int col0 = col > 0 ? 0 : 1;
			int row0 = row > 0 ? 0 : 1;
			int col1 = col < 2 ? 2 : 1;
			int row1 = row < 2 ? 2 : 1;

			float val00 = m[Index(row0, col0)];
			float val01 = m[Index(row0, col1)];
			float val10 = m[Index(row1, col0)];
			float val11 = m[Index(row1, col1)];

			int index = Index(row, col);
			float sign = (index & 1) != 0 ? -1.0f : 1.0f;
			cofactors[index] = (val00 * val11 - val01 * val10) * sign;
		}
	}

	float determinant = m[0] * cofactors[0] - m[1] * cofactors[1] + m[2] * cofactors[2];

	if (determinant == 0.0f)
		return Optional<Mat3x3f>();

	Mat3x3f result = cofactors.GetTransposed();
	MultiplyCells(result, 1.0f / determinant);
	return result;
}

TEST_CASE("Mat3x3f.GetInverse")
{
	Mat3x3f matrix;
	matrix[0] = 3.0f;
	matrix[1] = 0.0f;
	matrix[2] = 2.0f;
	matrix[3] = 2.0f;
	matrix[4] = 0.0f;
	matrix[5] = -2.0f;
	matrix[6] = 0.0f;
	matrix[7] = 1.0f;
	matrix[8] = 1.0f;

	Optional<Mat3x3f> inverseResult = matrix.GetInverse();

	CHECK(inverseResult.HasValue() == true);

	const Mat3x3f& inverse = inverseResult.GetValue();

	CHECK(inverse[0] == doctest::Approx(0.2f));
	CHECK(inverse[1] == doctest::Approx(0.2f));
	CHECK(inverse[2] == doctest::Approx(0.0f));
	CHECK(inverse[3] == doctest::Approx(-0.2f));
	CHECK(inverse[4] == doctest::Approx(0.3f));
	CHECK(inverse[5] == doctest::Approx(1.0f));
	CHECK(inverse[6] == doctest::Approx(0.2f));
	CHECK(inverse[7] == doctest::Approx(-0.3f));
	CHECK(inverse[8] == doctest::Approx(0.0f));
}

/* ======================== *
 * === STATIC FUNCTIONS === *
 * ======================== */

Mat3x3f Mat3x3f::RotateAroundAxis(Vec3f axis, float angle)
{
	axis.Normalize();

	const float xx = axis.x * axis.x;
	const float yy = axis.y * axis.y;
	const float zz = axis.z * axis.z;

	const float xy = axis.x * axis.y;
	const float xz = axis.x * axis.z;
	const float yz = axis.y * axis.z;

	const float ca = std::cos(angle);
	const float sa = std::sin(angle);

	Mat3x3f result;

	result[0] = xx + (1.0f - xx) * ca;
	result[1] = xy * (1.0f - ca) + axis.z * sa;
	result[2] = xz * (1.0f - ca) - axis.y * sa;

	result[3] = xy * (1.0f - ca) - axis.z * sa;
	result[4] = yy + (1.0f - yy) * ca;
	result[5] = yz * (1.0f - ca) + axis.x * sa;

	result[6] = xz * (1.0f - ca) + axis.y * sa;
	result[7] = yz * (1.0f - ca) - axis.x * sa;
	result[8] = zz + (1.0f - zz) * ca;

	return result;
}

Mat3x3f Mat3x3f::RotateEuler(const Vec3f& angles)
{
	Mat3x3f x, y, z;

	x[0] = 1.0f;
	x[1] = 0.0f;
	x[2] = 0.0f;
	x[3] = 0.0f;
	x[4] = std::cos(angles.x);
	x[5] = std::sin(angles.x);
	x[6] = 0.0f;
	x[7] = -std::sin(angles.x);
	x[8] = std::cos(angles.x);

	y[0] = std::cos(angles.y);
	y[1] = 0.0f;
	y[2] = -std::sin(angles.y);
	y[3] = 0.0f;
	y[4] = 1.0f;
	y[5] = 0.0f;
	y[6] = std::sin(angles.y);
	y[7] = 0.0f;
	y[8] = std::cos(angles.y);

	z[0] = std::cos(angles.z);
	z[1] = -std::sin(angles.z);
	z[2] = 0.0f;
	z[3] = std::sin(angles.z);
	z[4] = std::cos(angles.z);
	z[5] = 0.0f;
	z[6] = 0.0f;
	z[7] = 0.0f;
	z[8] = 1.0f;

	return y * x * z;
}

Mat3x3f Mat3x3f::FromAxes(const Vec3f& x, const Vec3f& y, const Vec3f& z)
{
	Mat3x3f result;

	result[0] = x.x;
	result[1] = x.y;
	result[2] = x.z;

	result[3] = y.x;
	result[4] = y.y;
	result[5] = y.z;

	result[6] = z.x;
	result[7] = z.y;
	result[8] = z.z;

	return result;
}

/* ======================== *
 * ====== OPERATORS ======= *
 * ======================== */

Mat3x3f operator*(const Mat3x3f& a, const Mat3x3f& b)
{
	Mat3x3f result;

	result[0] = a[0] * b[0] + a[3] * b[1] + a[6] * b[2];
	result[1] = a[1] * b[0] + a[4] * b[1] + a[7] * b[2];
	result[2] = a[2] * b[0] + a[5] * b[1] + a[8] * b[2];

	result[3] = a[0] * b[3] + a[3] * b[4] + a[6] * b[5];
	result[4] = a[1] * b[3] + a[4] * b[4] + a[7] * b[5];
	result[5] = a[2] * b[3] + a[5] * b[4] + a[8] * b[5];

	result[6] = a[0] * b[6] + a[3] * b[7] + a[6] * b[8];
	result[7] = a[1] * b[6] + a[4] * b[7] + a[7] * b[8];
	result[8] = a[2] * b[6] + a[5] * b[7] + a[8] * b[8];

	return result;
}

Vec3f operator*(const Mat3x3f& m, const Vec3f& v)
{
	return Vec3f(m[0] * v.x + m[3] * v.y + m[6] * v.z,
		m[1] * v.x + m[4] * v.y + m[7] * v.z,
		m[2] * v.x + m[5] * v.y + m[8] * v.z);
}

Vec3f operator*(const Vec3f& v, const Mat3x3f& m)
{
	return Vec3f(m[0] * v.x + m[1] * v.x + m[2] * v.x,
		m[3] * v.y + m[4] * v.y + m[5] * v.y,
		m[6] * v.z + m[7] * v.z + m[8] * v.z);
}

} // namespace kokko
