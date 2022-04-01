#pragma once

#include <filesystem>

#include "Core/Optional.hpp"
#include "Core/String.hpp"
#include "Core/StringView.hpp"

#include "EditorWindow.hpp"

class Allocator;

struct TextureId;

struct ImVec2;

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
	void DrawRootEntry(EditorContext& context, float columnWidth, const char* name);
	void DrawEntry(
		EditorContext& context,
		const std::filesystem::directory_entry& entry,
		float columnWidth,
		const char* overrideName);

	void DrawIconAndName(ImVec2 startPos, TextureId icon, float iconSize, const char* name);

	void MoveToPath(EditorContext& context, const std::filesystem::path& path);
	void SelectPath(EditorContext& context, const std::filesystem::path& path, bool editAsset);

	// Absolute path is any path that comes from std::filesystem::directory_entry
	Optional<String> AbsolutePathToVirtual(EditorContext& context, ConstStringView absolute);
	Optional<ConstStringView> AbsolutePathToRelative(EditorContext& context, ConstStringView absolute);
	String RelativePathToVirtual(ConstStringView path) const;

	Allocator* allocator;

	String currentVirtualRoot;
	String currentDirectory;
	String selectedPath;
	std::filesystem::path selectedPathFs; // Used to simplify checking for selected file

	const EditorImages* editorImages;

	String pathStore;
};

}
}
