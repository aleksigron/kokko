#pragma once

#include <cstdint>

#include "Core/Array.hpp"
#include "Core/HashMap.hpp"
#include "Core/Optional.hpp"
#include "Core/String.hpp"
#include "Core/StringView.hpp"
#include "Core/Uid.hpp"

class Allocator;

namespace kokko
{

class Filesystem;

namespace editor
{

class EditorProject;

enum class AssetType
{
	Level,
	Material,
	Model,
	Shader,
	Texture
};

class AssetInfo
{
public:
	AssetInfo(Allocator* allocator, ConstStringView virtualMount, ConstStringView relativePath,
		Uid uid, uint64_t contentHash, AssetType type);

	const String& GetVirtualPath() const { return virtualPath; }
	ConstStringView GetFilename() const { return filename; }
	Uid GetUid() const { return uid; }
	AssetType GetType() const { return type; }

private:
	friend class AssetLibrary;

	// Holds complete virtual path, lower members are referencing parts of this string
	String virtualPath;

	ConstStringView virtualMount;
	ConstStringView pathRelativeToMount;
	ConstStringView filename;

	Uid uid;
	uint64_t contentHash;
	AssetType type;
};

class AssetLibrary
{
public:
	AssetLibrary(Allocator* allocator, Filesystem* filesystem);
	~AssetLibrary();

	const AssetInfo* FindAssetByUid(const Uid& uid);
	const AssetInfo* FindAssetByVirtualPath(const String& virtualPath);

	Optional<Uid> CreateAsset(AssetType type, ConstStringView pathRelativeToAssets, ArrayView<const uint8_t> content);
	bool UpdateAssetContent(const Uid& uid, ArrayView<const uint8_t> content);

	bool ScanEngineAssets();
	void SetProject(const EditorProject* project);

private:
	bool ScanAssets(bool scanEngineAndEditor, bool scanProject);

	uint64_t CalculateHash(AssetType type, ArrayView<const uint8_t> content);

private:
	Allocator* allocator;
	Filesystem* filesystem;
	const EditorProject* editorProject;

	HashMap<Uid, uint32_t> uidToIndexMap;
	HashMap<String, uint32_t> pathToIndexMap;

	Array<AssetInfo> assets;
};

}
}
