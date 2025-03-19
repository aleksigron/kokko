#pragma once

#include "Core/String.hpp"

#include "Resources/AssetLoader.hpp"

namespace kokko
{

class Allocator;
class Filesystem;
class AssetLibrary;

class TestRunnerAssetLoader : public AssetLoader
{
public:
	TestRunnerAssetLoader(Allocator* allocator, Filesystem* filesystem, AssetLibrary* assetLibrary);

	virtual LoadResult LoadAsset(const Uid& uid, Array<uint8_t>& output) override;
	virtual Optional<Uid> GetAssetUidByVirtualPath(const ConstStringView& path) override;
	virtual Optional<String> GetAssetVirtualPath(const Uid& uid) override;

private:
	Filesystem* filesystem;
	AssetLibrary* assetLibrary;

	String pathString;
};

}