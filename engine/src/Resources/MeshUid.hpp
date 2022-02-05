#pragma once

#include <cstdint>

#include "Core/Uid.hpp"

namespace kokko
{

struct MeshUid
{
	Uid modelUid;
	uint32_t meshIndex;

	static const size_t StringLength = Uid::StringLength + sizeof(meshIndex) * 2 + 1;

	bool operator==(const MeshUid& other) const
	{
		return modelUid == other.modelUid && meshIndex == other.meshIndex;
	}

	bool operator!=(const MeshUid& other) const
	{
		return operator==(other) == false;
	}

	static Optional<MeshUid> FromString(ArrayView<const char> str);
	void WriteTo(ArrayView<char> out) const;
};

}
