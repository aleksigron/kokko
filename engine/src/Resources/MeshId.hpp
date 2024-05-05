#pragma once

#include <cstdint>

#include "Core/Uid.hpp"

namespace kokko
{

struct ModelId
{
	unsigned int i;

	bool operator==(ModelId other) const { return other.i == i; }
	bool operator!=(ModelId other) const { return operator==(other) == false; }

	static const ModelId Null;
};

struct MeshId
{
	ModelId modelId;
	uint32_t meshIndex;

	bool operator==(MeshId other) const { return modelId == other.modelId && meshIndex == other.meshIndex; }
	bool operator!=(MeshId other) const { return !operator==(other); }

	static const MeshId Null;
};

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

} // namespace kokko
