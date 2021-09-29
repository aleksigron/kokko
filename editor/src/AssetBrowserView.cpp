#include "AssetBrowserView.hpp"

#include "imgui.h"

#include "EditorConstants.hpp"
#include "EditorImages.hpp"

AssetBrowserView::AssetBrowserView() :
	EditorWindow("Asset Browser")
{
	currentPath = std::filesystem::current_path();
	selectedPath = std::filesystem::path();
}

AssetBrowserView::~AssetBrowserView()
{
}

void AssetBrowserView::Initialize(const EditorImages* editorImages)
{
	this->editorImages = editorImages;
}

void AssetBrowserView::Update(EditorContext& context)
{
	namespace fs = std::filesystem;

	if (windowIsOpen)
	{
		if (ImGui::Begin(windowTitle, &windowIsOpen))
		{
			std::string curPathStr = currentPath.u8string();

			// Directory path text

			if (ImGui::Button("Go to parent"))
			{
				// Move up a directory
				currentPath = currentPath.parent_path();
				selectedPath = fs::path();
			}

			ImGui::SameLine();

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
				for (fs::directory_iterator itr(currentPath), end; itr != end; ++itr, ++index)
				{
					ImGui::PushID(index);
					DrawEntry(itr, columnWidth);
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

void AssetBrowserView::DrawEntry(const std::filesystem::directory_iterator& entry, float columnWidth)
{
	namespace fs = std::filesystem;

	ImGui::TableNextColumn();

	bool isDir = entry->is_directory();
	bool isFile = entry->is_regular_file();

	const fs::path& path = entry->path();
	std::string fileStr = path.filename().u8string();

	ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_AllowDoubleClick;
	bool selected = path == selectedPath;

	ImVec2 buttonSize(columnWidth, columnWidth);
	ImVec2 cursorStartPos = ImGui::GetCursorPos();

	if (ImGui::Selectable("##AssetBrowserItem", selected, selectableFlags, buttonSize))
	{
		if (isFile)
		{
			// Select file
			selectedPath = path;
		}
		else if (isDir)
		{
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				// Move into directory
				currentPath = path;
				selectedPath = fs::path();
			}
			else
			{
				selectedPath = path;
			}
		}
	}

	if (isFile)
	{
		if (ImGui::BeginDragDropSource())
		{
			const char* type = EditorConstants::AssetDragDropType;

			std::string pathStr = path.u8string();
			ImGui::SetDragDropPayload(type, pathStr.c_str(), pathStr.size());

			ImGui::Text("%s", fileStr.c_str());

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

void AssetBrowserView::UpdateDirectoryListing()
{
}
