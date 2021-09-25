#include "Math/Mat4x4.hpp"

#include <immintrin.h>

#include "Math/Vec2.hpp"

#define KOKKO_USE_SSE

Mat4x4f Mat4x4f::ScreenSpaceProjection(const Vec2<int>& screenSize)
{
	const float far = 100.0f;
	const float near = -100.0f;

	Mat4x4f result;
	result[0] = 2.0f / screenSize.x;
	result[5] = 2.0f / -screenSize.y;
	result[10] = -2.0f / (far - near);
	result[12] = -1.0f;
	result[13] = 1.0f;
	result[14] = -(far + near) / (far - near);

	return result;
}

void Mat4x4f::MultiplyMany(unsigned int count, const Mat4x4f* a, const Mat4x4f* b, Mat4x4f* out)
{
#ifdef KOKKO_USE_SSE
	for (unsigned int i = 0; i < count; ++i)
	{
		const float* aPtr = a[i].m;
		const float* bRowPtr = b[i].m;
		float* resultRowPtr = out[i].m;

		const __m128 ax = _mm_load_ps(aPtr + 0);
		const __m128 ay = _mm_load_ps(aPtr + 4);
		const __m128 az = _mm_load_ps(aPtr + 8);
		const __m128 aw = _mm_load_ps(aPtr + 12);

		for (unsigned int j = 0; j < 4; ++j, bRowPtr += 4, resultRowPtr += 4)
		{
			__m128 bx = _mm_set_ps1(bRowPtr[0]);
			__m128 by = _mm_set_ps1(bRowPtr[1]);
			__m128 bz = _mm_set_ps1(bRowPtr[2]);
			__m128 bw = _mm_set_ps1(bRowPtr[3]);

			__m128 x = _mm_mul_ps(ax, bx);
			__m128 y = _mm_mul_ps(ay, by);
			__m128 z = _mm_mul_ps(az, bz);
			__m128 w = _mm_mul_ps(aw, bw);

			__m128 rxy = _mm_add_ps(x, y);
			__m128 rzw = _mm_add_ps(z, w);
			__m128 r = _mm_add_ps(rxy, rzw);

			_mm_store_ps(resultRowPtr, r);
		}
	}
#else
	for (unsigned int i = 0; i < count; ++i)
		out[i] = a[i] * b[i];
#endif
}

void Mat4x4f::MultiplyOneByMany(const Mat4x4f& a, unsigned int count, const Mat4x4f* b, Mat4x4f* out)
{
#ifdef KOKKO_USE_SSE
	const __m128 ax = _mm_load_ps(a.m + 0);
	const __m128 ay = _mm_load_ps(a.m + 4);
	const __m128 az = _mm_load_ps(a.m + 8);
	const __m128 aw = _mm_load_ps(a.m + 12);

	for (unsigned int i = 0; i < count; ++i)
	{
		const float* bRowPtr = b[i].m;
		float* resultRowPtr = out[i].m;

		for (unsigned int j = 0; j < 4; ++j, bRowPtr += 4, resultRowPtr += 4)
		{
			__m128 bx = _mm_set_ps1(bRowPtr[0]);
			__m128 by = _mm_set_ps1(bRowPtr[1]);
			__m128 bz = _mm_set_ps1(bRowPtr[2]);
			__m128 bw = _mm_set_ps1(bRowPtr[3]);

			__m128 x = _mm_mul_ps(ax, bx);
			__m128 y = _mm_mul_ps(ay, by);
			__m128 z = _mm_mul_ps(az, bz);
			__m128 w = _mm_mul_ps(aw, bw);

			__m128 rxy = _mm_add_ps(x, y);
			__m128 rzw = _mm_add_ps(z, w);
			__m128 r = _mm_add_ps(rxy, rzw);

			_mm_store_ps(resultRowPtr, r);
		}
	}
#else
	for (unsigned int i = 0; i < count; ++i)
		out[i] = a * b[i];
#endif
}