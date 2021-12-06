#pragma once

#include <cstdint>

#include "Core/Array.hpp"
#include "Core/HashMap.hpp"
#include "Core/Optional.hpp"
#include "Core/Uid.hpp"

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
	AssetLibrary(Allocator* allocator, Filesystem* filesystem);
	~AssetLibrary();

	void Initialize(const EditorProject* project);
	void ScanAssets();

private:
	struct AssetArrayRef
	{
		uint32_t assetIndex;
	};

private:
	Allocator* allocator;
	Filesystem* filesystem;
	const EditorProject* editorProject;
	HashMap<Uid, AssetArrayRef> uidMap;
};

}
}
