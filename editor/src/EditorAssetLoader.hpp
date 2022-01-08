#pragma once

#include "Core/String.hpp"

#include "Resources/AssetLoader.hpp"

class Allocator;
class Filesystem;

namespace kokko
{
namespace editor
{

class AssetLibrary;

class EditorAssetLoader : public AssetLoader
{
public:
	EditorAssetLoader(Allocator* allocator, Filesystem* filesystem, AssetLibrary* assetLibrary);

	virtual bool LoadAsset(const Uid& uid, Array<uint8_t>& output) override;
	virtual Optional<Uid> GetAssetUidByVirtualPath(const StringRef& path) override;
	virtual Optional<String> GetAssetVirtualPath(const Uid& uid) override;

private:
	Filesystem* filesystem;
	AssetLibrary* assetLibrary;

	String pathString;
};

}
}
