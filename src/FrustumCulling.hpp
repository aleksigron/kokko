#pragma once

struct BoundingBox;
struct ViewFrustum;
struct CullStatePacked16;

namespace FrustumCulling
{
	void CullAABB(const ViewFrustum* frustum,
				  unsigned int count,
				  const BoundingBox* bounds,
				  CullStatePacked16* state);
}
