#pragma once

#include <cstddef>

#include "Math/Mat4x4.hpp"
#include "Math/Vec4.hpp"
#include "Math/Vec3.hpp"
#include "Math/Vec2.hpp"

template <typename Type, size_t Count>
class UniformBlockArray
{
private:
	struct alignas(16) AlignWrapper
	{
		Type value;
	};

	AlignWrapper values[Count];

public:
	Type& operator[](size_t index) { return values[index].value; }
	const Type& operator[](size_t index) const { return values[index].value; }
};

struct LightingUniformBlock
{
	static constexpr size_t MaxLightCount = 8;
	static constexpr size_t MaxCascadeCount = 4;
	static const unsigned int BindingPoint = 0;

	UniformBlockArray<Vec3f, MaxLightCount> lightColors;
	UniformBlockArray<Vec3f, MaxLightCount> lightPositions;
	UniformBlockArray<Vec4f, MaxLightCount> lightDirections;

	UniformBlockArray<Mat4x4f, MaxCascadeCount> shadowMatrices;
	UniformBlockArray<float, MaxCascadeCount + 1> shadowSplits;

	alignas(16) Mat4x4f perspectiveMatrix;
	alignas(16) Vec3f ambientColor;
	alignas(8) Vec2f halfNearPlane;
	alignas(8) Vec2f shadowMapScale;
	alignas(8) Vec2f frameResolution;

	alignas(4) int pointLightCount;
	alignas(4) int spotLightCount;
	alignas(4) int cascadeCount;

	alignas(4) float shadowBiasOffset;
	alignas(4) float shadowBiasFactor;
	alignas(4) float shadowBiasClamp;
};

struct ViewportUniformBlock
{
	static const unsigned int BindingPoint = 1;

	alignas(16) Mat4x4f VP;
	alignas(16) Mat4x4f V;
	alignas(16) Mat4x4f P;
};

struct TransformUniformBlock
{
	static const unsigned int BindingPoint = 2;

	alignas(16) Mat4x4f MVP;
	alignas(16) Mat4x4f MV;
	alignas(16) Mat4x4f M;
};

struct MaterialUniformBlock
{
	static const unsigned int BindingPoint = 3;
};
