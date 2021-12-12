#pragma once

#include <cstdint>
#include <filesystem>

#include "Core/Array.hpp"
#include "Core/HashMap.hpp"
#include "Core/Optional.hpp"
#include "Core/Uid.hpp"

#include "Hash_StdFilesystem.hpp"

class Allocator;
class Filesystem;

namespace kokko
{
namespace editor
{

class EditorProject;

enum class AssetType : uint8_t
{
	Material
};

class AssetLibrary
{
public:
	struct AssetInfo
	{
		std::filesystem::path path;
		Uid uid;
		AssetType type;
		uint32_t arrayIndex;
	};

public:
	AssetLibrary(Allocator* allocator, Filesystem* filesystem);
	~AssetLibrary();

	void Initialize(const EditorProject* project);
	void ScanAssets();

	const AssetInfo* FindAssetByPath(const std::filesystem::path& path);

private:
	struct MaterialInfo
	{
	};

private:
	Allocator* allocator;
	Filesystem* filesystem;
	const EditorProject* editorProject;

	HashMap<Uid, uint32_t> uidToIndexMap;
	HashMap<std::filesystem::path, uint32_t> pathToIndexMap;

	Array<AssetInfo> assets;

	Array<MaterialInfo> materials;
};

}
}
