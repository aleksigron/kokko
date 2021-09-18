#pragma once

#include <cstdint>

#include "Core/ArrayView.hpp"

class MeshManager;
struct MeshId;

class MeshLoader
{
private:
	MeshManager* meshManager;

public:
	enum class Status
	{
		Success,
		NoData,
		FileMagicDoesNotMatch,
		FileVersionIncompatible,
		FileSizeDoesNotMatch,
	};

	MeshLoader(MeshManager* meshManager);

	Status LoadFromBuffer(MeshId meshId, ArrayView<uint8_t> buffer);
};
