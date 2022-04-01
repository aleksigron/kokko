#pragma once

#include <cstdint>

#include "Core/Array.hpp"
#include "Core/Optional.hpp"
#include "Core/StringView.hpp"

namespace kokko
{

class String;
struct Uid;

class AssetLoader
{
public:
	virtual bool LoadAsset(const Uid& uid, Array<uint8_t>& output) = 0;
	virtual Optional<Uid> GetAssetUidByVirtualPath(const ConstStringView& path) = 0;
	virtual Optional<String> GetAssetVirtualPath(const Uid& uid) = 0;
};

}
