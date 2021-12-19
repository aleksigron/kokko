#pragma once

#include <cstdint>
#include <filesystem>

#include "Core/Array.hpp"
#include "Core/HashMap.hpp"
#include "Core/Optional.hpp"
#include "Core/String.hpp"
#include "Core/Uid.hpp"

#include "Hash_StdFilesystem.hpp"

class Allocator;
class Filesystem;

namespace kokko
{
namespace editor
{

class EditorProject;

enum class AssetType
{
	Material
};

struct AssetInfo
{
	StringRef virtualPath;
	String filePath;
	Uid uid;
	AssetType type;

	String GetVirtualPath() const;
};

class AssetLibrary
{
public:

public:
	AssetLibrary(Allocator* allocator, Filesystem* filesystem);
	~AssetLibrary();

	const AssetInfo* FindAssetByUid(const Uid& uid);
	const AssetInfo* FindAssetByVirtualPath(const String& virtualPath);

	void ScanEngineAssets();
	void SetProject(const EditorProject* project);

private:
	void ScanAssets(bool scanProject);

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
