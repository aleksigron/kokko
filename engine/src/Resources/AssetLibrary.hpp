#pragma once

#include <cstdint>
#include <filesystem>

#include "Core/Array.hpp"
#include "Core/HashMap.hpp"
#include "Core/Optional.hpp"
#include "Core/SortedArray.hpp"
#include "Core/String.hpp"
#include "Core/StringView.hpp"
#include "Core/Uid.hpp"

#include "Resources/AssetType.hpp"

namespace kokko
{

class Allocator;
class Filesystem;

struct AssetScopeConfiguration
{
	std::filesystem::path assetFolderPath;
	String virtualMountName;
};

class AssetInfo
{
public:
	AssetInfo(Allocator* allocator, ConstStringView virtualMount, ConstStringView relativePath,
		Uid uid, uint64_t contentHash, int32_t metadataIndex, AssetType type);

	void UpdateFilename(ConstStringView newFilename);

	const String& GetVirtualPath() const { return virtualPath; }
	ConstStringView GetRelativeFolderPath() const { return relativeFolderPath; }
	ConstStringView GetRelativeFilePath() const { return relativeFilePath; }
	ConstStringView GetFilename() const { return filename; }
	Uid GetUid() const { return uid; }
	AssetType GetType() const { return type; }

private:
	friend class AssetLibrary;

	// Holds complete virtual path, string view members are referencing parts of this string
	String virtualPath;

	ConstStringView virtualMount;
	ConstStringView relativeFolderPath; // Relative to the mounted folder
	ConstStringView relativeFilePath; // Relative to the mounted folder
	ConstStringView filename;

	Uid uid;
	uint64_t contentHash;
	int32_t metadataIndex;
	AssetType type;
};

class AssetLibrary
{
public:
	AssetLibrary(Allocator* allocator, Filesystem* filesystem);
	~AssetLibrary();

	const AssetInfo* FindAssetByUid(const Uid& uid);
	const AssetInfo* FindAssetByVirtualPath(const String& virtualPath);

	// Creating a new asset invalidates any pointers obtained from FindAsset*() or Get*Metadata()
	Optional<Uid> CreateAsset(AssetType type, ConstStringView pathRelativeToAssets, ArrayView<const uint8_t> content);
	bool RenameAsset(const Uid& uid, ConstStringView newFilename);
	bool UpdateAssetContent(const Uid& uid, ArrayView<const uint8_t> content);

	const TextureAssetMetadata* GetTextureMetadata(const AssetInfo* asset) const;
	bool UpdateTextureMetadata(const Uid& uid, const TextureAssetMetadata& metadata);

	void SetAppScopeConfig(const AssetScopeConfiguration& config);
	void SetProjectScopeConfig(const AssetScopeConfiguration& config);

	bool ScanAssets(bool scanEngine, bool scanApp, bool scanProject);

	bool GetNextUpdatedAssetUid(AssetType typeFilter, Uid& uid);

private:
	uint64_t CalculateHash(AssetType type, ArrayView<const uint8_t> content);

private:
	Allocator* allocator;
	Filesystem* filesystem;

	HashMap<Uid, uint32_t> uidToIndexMap;
	HashMap<String, uint32_t> pathToIndexMap;

	Array<AssetInfo> assets;
	Array<TextureAssetMetadata> textureMetadata;
	SortedArray<Uid> updatedAssets;
	AssetScopeConfiguration applicationConfig;
	AssetScopeConfiguration projectConfig;
};

}
