#pragma once

struct BoundingBox;
struct ViewFrustum;

namespace FrustumCulling
{
	enum class CullingState : unsigned char
	{
		Outside = 0,
		Intersect = 1,
		Inside = 2
	};

	void CullAABB(const ViewFrustum* frustum,
				  unsigned int boxCount,
				  const BoundingBox* boxes,
				  CullingState* state);
}
