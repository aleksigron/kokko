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

AssetLoader::LoadResult TestRunnerAssetLoader::LoadAsset(const Uid& uid, Array<uint8_t>& output)
{
	if (auto asset = assetLibrary->FindAssetByUid(uid))
	{
		LoadResult result;
		result.assetType = asset->GetType();
		if (result.assetType == AssetType::Texture)
		{
			const TextureAssetMetadata* metadata = assetLibrary->GetTextureMetadata(asset);
			result.metadataSize = static_cast<uint32_t>(sizeof(TextureAssetMetadata));
			result.assetStart = Math::RoundUpToMultiple(result.metadataSize, 16u);

			output.Resize(result.assetStart);
			memcpy(output.GetData(), metadata, result.metadataSize);
		}

		const String& pathStr = asset->GetVirtualPath();
		if (filesystem->ReadBinary(pathStr.GetCStr(), output))
		{
			result.success = true;
			result.assetSize = output.GetCount() - result.assetStart;
			return result;
		}
	}

	return LoadResult();
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
