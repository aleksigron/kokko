#include "Editor/AssetBrowserView.hpp"

#include "imgui.h"

#include "Editor/EditorImages.hpp"
#include "Editor/EditorWindowInfo.hpp"

AssetBrowserView::AssetBrowserView()
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

void AssetBrowserView::Draw(EditorWindowInfo& windowInfo)
{
	namespace fs = std::filesystem;

	if (windowInfo.isOpen)
	{
		if (ImGui::Begin(windowInfo.title, &windowInfo.isOpen))
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
			float buttonSide = fontSize * 9.0f;
			float cellWidth = buttonSide + style.CellPadding.x * 2.0f;
			ImVec2 buttonSize(buttonSide, buttonSide);
			float availableWidth = ImGui::GetContentRegionAvail().x - scrollbarWidth;
			int columnCount = static_cast<int>(availableWidth / cellWidth);

			ImGuiTableFlags tableFlags = ImGuiTableFlags_ScrollY;

			if (ImGui::BeginTable("AssetBrowserTable", columnCount, tableFlags))
			{
				for (int col = 0; col < columnCount; ++col)
				{
					ImGuiTableColumnFlags flags = ImGuiTableColumnFlags_WidthFixed;

					float columnWidth;
					if (col + 1 == columnCount)
						columnWidth = 0.0f;
					else
						columnWidth = buttonSide;

					ImGui::PushID(col);
					ImGui::TableSetupColumn("Column", flags, columnWidth);
					ImGui::PopID();
				}

				size_t index = 1;
				for (fs::directory_iterator itr(currentPath), end; itr != end; ++itr, ++index)
				{
					ImGui::TableNextColumn();

					bool isDir = itr->is_directory();
					bool isFile = itr->is_regular_file();

					fs::path path = itr->path();
					std::string pathStr = path.filename().u8string();

					ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_AllowDoubleClick;
					bool selected = path == selectedPath;

					ImVec2 cursorStartPos = ImGui::GetCursorPos();

					ImGui::PushID(index);
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

					TextureId texId = isDir ? editorImages->folderIcon : editorImages->genericFileIcon;
					void* image = editorImages->GetImGuiTextureId(texId);

					ImGui::SetCursorPos(cursorStartPos);
					ImGui::Image(image, buttonSize);

					ImGui::TextWrapped(pathStr.c_str());
					ImGui::Spacing();
					ImGui::PopID();
				}

				ImGui::EndTable();
			}
		}

		if (windowInfo.requestFocus)
			ImGui::SetWindowFocus();

		ImGui::End();
	}
}

void AssetBrowserView::UpdateDirectoryListing()
{
}
