#include "AssetBrowserView.hpp"

#include "imgui.h"

#include "Core/Core.hpp"

#include "AssetLibrary.hpp"
#include "EditorConstants.hpp"
#include "EditorContext.hpp"
#include "EditorImages.hpp"
#include "EditorProject.hpp"

namespace kokko
{
namespace editor
{

AssetBrowserView::AssetBrowserView(Allocator* allocator) :
	EditorWindow("Asset Browser"),
	allocator(allocator),
	currentVirtualPath(EditorConstants::VirtualPathAssets),
	editorImages(nullptr),
	pathStore(allocator)
{
}

AssetBrowserView::~AssetBrowserView()
{
}

void AssetBrowserView::Initialize(const EditorImages* editorImages)
{
	this->editorImages = editorImages;
}

void AssetBrowserView::OnEditorProjectChanged(const EditorContext& context)
{
	currentDirectory = std::filesystem::path();
	selectedPath = std::filesystem::path();
}

void AssetBrowserView::Update(EditorContext& context)
{
	namespace fs = std::filesystem;

	if (windowIsOpen)
	{
		if (ImGui::Begin(windowTitle, &windowIsOpen))
		{
			// TODO: Allow user to select engine resources or project assets folders at the folder first level

			const fs::path& root = context.project->GetAssetPath();

			if (ImGui::Button("Go to parent") && currentDirectory != root)
			{
				MoveToPath(context, currentDirectory.parent_path());
			}

			ImGui::SameLine();

			std::string curPathStr = currentDirectory.u8string();

			ImGuiInputTextFlags dirTextFlags = ImGuiInputTextFlags_ReadOnly;
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::InputText("##AssetBrowserPath", curPathStr.data(), curPathStr.length(), dirTextFlags);

			ImGuiStyle& style = ImGui::GetStyle();
			float scrollbarWidth = style.ScrollbarSize;

			float fontSize = ImGui::GetFontSize();
			float columnWidth = fontSize * 8.0f;
			float cellWidth = columnWidth + style.CellPadding.x * 2.0f;

			float availableWidth = ImGui::GetContentRegionAvail().x - scrollbarWidth;
			int columnCount = static_cast<int>(availableWidth / cellWidth);

			if (columnCount < 1)
				columnCount = 1;

			if (ImGui::BeginTable("AssetBrowserTable", columnCount, ImGuiTableFlags_ScrollY))
			{
				SetUpColumns(columnCount, columnWidth);

				int index = 1;
				fs::path currentDirAbsolute = context.project->GetAssetPath() / currentDirectory;
				for (fs::directory_iterator itr(currentDirAbsolute), end; itr != end; ++itr, ++index)
				{
					ImGui::PushID(index);
					DrawEntry(context, itr, columnWidth);
					ImGui::PopID();
				}

				ImGui::EndTable();
			}
		}

		if (requestFocus)
			ImGui::SetWindowFocus();

		ImGui::End();
	}
}

void AssetBrowserView::SetUpColumns(int columnCount, float columnWidth)
{
	for (int col = 0; col < columnCount; ++col)
	{
		ImGuiTableColumnFlags flags = ImGuiTableColumnFlags_WidthFixed;

		float width;
		if (col + 1 == columnCount)
			width = 0.0f;
		else
			width = columnWidth;

		ImGui::PushID(col);
		ImGui::TableSetupColumn("Column", flags, width);
		ImGui::PopID();
	}
}

void AssetBrowserView::DrawEntry(
	EditorContext& context,
	const std::filesystem::directory_iterator& entry,
	float columnWidth)
{
	namespace fs = std::filesystem;

	bool isDir = entry->is_directory();
	bool isFile = entry->is_regular_file();

	if (isFile && entry->path().extension() == EditorConstants::MetadataExtension)
		return;

	ImGui::TableNextColumn();
	
	auto relativeResult = MakePathRelative(context, entry->path());
	if (relativeResult.HasValue() == false)
		return;

	const std::filesystem::path& relativePath = relativeResult.GetValue();

	ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_AllowDoubleClick;
	bool selected = relativePath == selectedPath;

	ImVec2 buttonSize(columnWidth, columnWidth);
	ImVec2 cursorStartPos = ImGui::GetCursorPos();

	if (ImGui::Selectable("##AssetBrowserItem", selected, selectableFlags, buttonSize))
	{
		if (isFile)
		{
			SelectPath(context, relativePath);
		}
		else if (isDir)
		{
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				MoveToPath(context, relativePath);
			}
			else
			{
				SelectPath(context, relativePath);
			}
		}
	}

	std::string fileStr = relativePath.filename().u8string();

	if (isFile)
	{
		if (ImGui::BeginDragDropSource())
		{
			auto relativeResult = MakePathRelative(context, entry->path());
			if (relativeResult.HasValue())
			{
				const std::filesystem::path& relativePath = relativeResult.GetValue();

				if (auto asset = context.assetLibrary->FindAssetByVirtualPath(ConvertPath(relativePath)))
				{
					ImGui::SetDragDropPayload(EditorConstants::AssetDragDropType, &asset->uid, sizeof(Uid));

					ImGui::Text("%s", fileStr.c_str());
				}
			}

			ImGui::EndDragDropSource();
		}
	}

	TextureId texId = isDir ? editorImages->folderIcon : editorImages->genericFileIcon;
	void* image = editorImages->GetImGuiTextureId(texId);

	ImGui::SetCursorPos(cursorStartPos);
	ImGui::Image(image, buttonSize);

	ImGui::TextWrapped("%s", fileStr.c_str());
	ImGui::Spacing();
}

void AssetBrowserView::MoveToPath(EditorContext& context, const std::filesystem::path& path)
{
	currentDirectory = path;
	selectedPath = std::filesystem::path();

	context.selectedAsset = Optional<Uid>();
}

void AssetBrowserView::SelectPath(EditorContext& context, const std::filesystem::path& path)
{
	selectedPath = path;

	if (path.empty() == false)
	{
		auto asset = context.assetLibrary->FindAssetByVirtualPath(ConvertPath(path));

		if (asset != nullptr)
		{
			context.selectedAsset = asset->uid;
			return;
		}
	}

	context.selectedAsset = Optional<Uid>();
}

const String& AssetBrowserView::ConvertPath(const std::filesystem::path& from)
{
	std::string pathStr = from.generic_u8string();
	
	pathStore.Clear();
	pathStore.Append(currentVirtualPath);
	pathStore.Append('/');
	pathStore.Append(StringRef(pathStr.c_str(), pathStr.length()));

	return pathStore;
}

Optional<std::filesystem::path> AssetBrowserView::MakePathRelative(const EditorContext& context, const std::filesystem::path& absolute)
{
	std::error_code err;
	std::filesystem::path relativePath = std::filesystem::relative(absolute, context.project->GetAssetPath(), err);

	if (!err)
	{
		return relativePath;
	}
	else
	{
		KK_LOG_ERROR("std::filesystem::relative failed. Message: {}", err.message().c_str());
		return Optional<std::filesystem::path>();
	}
}

}
}
