#include "EditorAssetLoader.hpp"

#include "System/Filesystem.hpp"

#include "AssetLibrary.hpp"

namespace kokko
{
namespace editor
{

EditorAssetLoader::EditorAssetLoader(Allocator* allocator, Filesystem* filesystem, AssetLibrary* assetLibrary) :
	filesystem(filesystem),
	assetLibrary(assetLibrary),
	pathString(allocator)
{
}

bool EditorAssetLoader::LoadAsset(const Uid& uid, Array<uint8_t>& output)
{
	auto asset = assetLibrary->FindAssetByUid(uid);
	if (asset != nullptr)
	{
		auto pathStr = asset->GetVirtualPath();
		if (filesystem->ReadBinary(pathStr.GetCStr(), output))
		{
			return true;
		}
	}

	return false;
}

Optional<Uid> EditorAssetLoader::GetAssetUidByVirtualPath(const StringRef& path)
{
	pathString.Assign(path);

	if (auto asset = assetLibrary->FindAssetByVirtualPath(pathString))
		return asset->uid;

	return Optional<Uid>();
}

}
}
