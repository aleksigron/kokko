#include "TestRunnerAssetLoader.hpp"

#include "Resources/AssetLibrary.hpp"

#include "System/Filesystem.hpp"

namespace kokko
{

TestRunnerAssetLoader::TestRunnerAssetLoader(Allocator* allocator, Filesystem* filesystem, AssetLibrary* assetLibrary) :
	filesystem(filesystem),
	assetLibrary(assetLibrary),
	pathString(allocator)
{
}

bool TestRunnerAssetLoader::LoadAsset(const Uid& uid, Array<uint8_t>& output)
{
	if (auto asset = assetLibrary->FindAssetByUid(uid))
	{
		auto pathStr = asset->GetVirtualPath();
		if (filesystem->ReadBinary(pathStr.GetCStr(), output))
		{
			return true;
		}
	}

	return false;
}

Optional<Uid> TestRunnerAssetLoader::GetAssetUidByVirtualPath(const ConstStringView& path)
{
	pathString.Assign(path);

	if (auto asset = assetLibrary->FindAssetByVirtualPath(pathString))
		return asset->GetUid();

	return Optional<Uid>();
}

Optional<String> TestRunnerAssetLoader::GetAssetVirtualPath(const Uid& uid)
{
	if (auto asset = assetLibrary->FindAssetByUid(uid))
		return asset->GetVirtualPath();

	return Optional<String>();
}

}
