#pragma once

#include <cstdint>

#include "Core/Array.hpp"
#include "Core/Optional.hpp"
#include "Core/StringView.hpp"

#include "Resources/AssetType.hpp"

namespace kokko
{

class String;
struct Uid;

class AssetLoader
{
public:
	struct LoadResult
	{
		bool success = false;
		AssetType assetType = AssetType::Unknown;
		uint32_t metadataSize = 0;
		uint32_t assetStart = 0;
		uint32_t assetSize = 0;
	};

	virtual LoadResult LoadAsset(const Uid& uid, Array<uint8_t>& output) = 0;
	virtual Optional<Uid> GetAssetUidByVirtualPath(const ConstStringView& path) = 0;
	virtual Optional<String> GetAssetVirtualPath(const Uid& uid) = 0;

	virtual bool GetNextUpdatedAssetUid(AssetType typeFilter, Uid& uid) { return false; }
};

}
