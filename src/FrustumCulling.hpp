#pragma once

struct BoundingBox;
struct ViewFrustum;
struct BitPack;

namespace FrustumCulling
{
	void CullAABB(
		const ViewFrustum& frustum,
		unsigned int count,
		const BoundingBox* bounds,
		BitPack* visibility);
}
