#pragma once

#include "Math/Mat4x4.hpp"
#include "Math/Projection.hpp"

namespace kokko
{

struct CameraParameters
{
	Mat4x4fBijection transform;
	ProjectionParameters projection;
};

} // namespace kokko
