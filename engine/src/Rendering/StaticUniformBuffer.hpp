#pragma once

#include <cstddef>

#include "Math/Mat4x4.hpp"
#include "Math/Vec4.hpp"
#include "Math/Vec3.hpp"
#include "Math/Vec2.hpp"

namespace kokko
{

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

} // namespace kokko
