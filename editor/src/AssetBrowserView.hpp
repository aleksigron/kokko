#pragma once

#include <filesystem>

#include "Core/Optional.hpp"
#include "Core/String.hpp"
#include "Core/StringRef.hpp"

#include "EditorWindow.hpp"

class Allocator;

namespace kokko
{
namespace editor
{

class EditorImages;

struct EditorWindowInfo;

class AssetBrowserView : public EditorWindow
{
public:
	AssetBrowserView(Allocator* allocator);
	~AssetBrowserView();

	void Initialize(const EditorImages* editorImages);

	virtual void OnEditorProjectChanged(const EditorContext& context) override;

	virtual void Update(EditorContext& context) override;

private:
	void SetUpColumns(int columnCount, float columnWidth);
	void DrawEntry(EditorContext& context, const std::filesystem::directory_iterator& entry, float columnWidth);

	void MoveToPath(EditorContext& context, const std::filesystem::path& path);
	void SelectPath(EditorContext& context, const std::filesystem::path& path, bool editAsset);

	const String& ConvertPath(const std::filesystem::path& relativePath);
	Optional<std::filesystem::path> MakePathRelative(
		const EditorContext& context, const std::filesystem::path& absolute);

	Allocator* allocator;

	StringRef currentVirtualPath;
	std::filesystem::path currentDirectory;
	std::filesystem::path selectedPath;

	const EditorImages* editorImages;

	String pathStore;
};

}
}
