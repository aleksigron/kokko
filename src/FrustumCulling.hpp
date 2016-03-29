#pragma once

struct BoundingBox;
struct ViewFrustum;

namespace FrustumCulling
{
	void CullAABB(const ViewFrustum* frustum,
				  unsigned int boxCount,
				  const BoundingBox* boxes,
				  unsigned char* state);
}
