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

namespace UniformBlockBinding
{
	static constexpr unsigned int Global = 0;
	static constexpr unsigned int Viewport = 1;
	static constexpr unsigned int Material = 2;
	static constexpr unsigned int Object = 3;
};

struct LightingUniformBlock
{
	static constexpr size_t MaxLightCount = 8;
	static constexpr size_t MaxCascadeCount = 4;

	UniformBlockArray<Vec3f, MaxLightCount> lightColors;
	UniformBlockArray<Vec4f, MaxLightCount> lightPositions; // xyz: position, w: inverse square radius
	UniformBlockArray<Vec4f, MaxLightCount> lightDirections; // xyz: direction, w: spot light angle

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
	alignas(16) Mat4x4f VP;
	alignas(16) Mat4x4f V;
	alignas(16) Mat4x4f P;
};

struct TransformUniformBlock
{
	alignas(16) Mat4x4f MVP;
	alignas(16) Mat4x4f MV;
	alignas(16) Mat4x4f M;
};
