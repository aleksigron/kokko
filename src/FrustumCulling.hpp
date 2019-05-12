#pragma once

struct BoundingBox;
struct FrustumPlanes;
struct BitPack;

namespace FrustumCulling
{
	void CullAABB(
		const FrustumPlanes& frustum,
		unsigned int count,
		const BoundingBox* bounds,
		BitPack* visibility);
}
