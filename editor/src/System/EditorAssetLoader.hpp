#pragma once

#include "Core/String.hpp"

#include "Resources/AssetLoader.hpp"

namespace kokko
{

class Allocator;
class AssetLibrary;
class Filesystem;

namespace editor
{

class EditorAssetLoader : public AssetLoader
{
public:
	EditorAssetLoader(Allocator* allocator, Filesystem* filesystem, AssetLibrary* assetLibrary);

	virtual LoadResult LoadAsset(const Uid& uid, Array<uint8_t>& output) override;
	virtual Optional<Uid> GetAssetUidByVirtualPath(const ConstStringView& path) override;
	virtual Optional<String> GetAssetVirtualPath(const Uid& uid) override;

	virtual bool GetNextUpdatedAssetUid(AssetType typeFilter, Uid& uid) override;

private:
	Filesystem* filesystem;
	AssetLibrary* assetLibrary;

	String pathString;
};

}
}
