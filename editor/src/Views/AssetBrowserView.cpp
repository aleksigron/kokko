#include "AssetBrowserView.hpp"

#include "imgui.h"

#include "Core/Core.hpp"

#include "Engine/EngineConstants.hpp"

#include "Resources/AssetLibrary.hpp"

#include "System/FilesystemResolver.hpp"

#include "App/EditorConstants.hpp"
#include "App/EditorContext.hpp"
#include "App/EditorImages.hpp"
#include "App/EditorProject.hpp"

namespace kokko
{
namespace editor
{

AssetBrowserView::AssetBrowserView(Allocator* allocator) :
	EditorWindow("Asset Browser"),
	allocator(allocator),
	currentVirtualRoot(allocator),
	currentDirectory(allocator),
	selectedPath(allocator),
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
	currentVirtualRoot.Clear();
	currentDirectory.Clear();
	selectedPath.Clear();
}

void AssetBrowserView::Update(EditorContext& context)
{
	namespace fs = std::filesystem;

	if (windowIsOpen)
	{
		ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);

		if (ImGui::Begin(windowTitle, &windowIsOpen))
		{
			if (context.project != nullptr)
			{
				bool createEnabled = currentVirtualRoot == "assets";

				const char* const createAssetText = "Create asset...";
				ImVec2 textSize = ImGui::CalcTextSize(createAssetText);
				float width = textSize.x + 2.0f * ImGui::GetStyle().FramePadding.x;
				ImGui::SetNextItemWidth(width);
				
				if (!createEnabled)
					ImGui::BeginDisabled();

				if (ImGui::BeginCombo("##CreateEntityCombo", createAssetText, ImGuiComboFlags_NoArrowButton))
				{
					if (ImGui::Selectable("Material"))
					{
						CreateMaterial(context);
					}

					ImGui::EndCombo();
				}

				if (!createEnabled)
					ImGui::EndDisabled();

				ImGui::SameLine();

				if (ImGui::Button("Go to parent"))
				{
					if (currentDirectory.GetLength() == 0)
						currentVirtualRoot.Clear();
					else
					{
						intptr_t slash = currentDirectory.GetRef().FindLast(ConstStringView("/"));

						if (slash > 0)
							currentDirectory.Resize(slash);
						else
							currentDirectory.Clear();
					}
					selectedPath.Clear();
					selectedPathFs.clear();
				}

				ImGui::SameLine();

				String curDirStr(allocator);
				curDirStr.Append(currentVirtualRoot);
				if (currentDirectory.GetLength() != 0)
				{
					curDirStr.Append('/');
					curDirStr.Append(currentDirectory);
				}

				ImGuiInputTextFlags dirTextFlags = ImGuiInputTextFlags_ReadOnly;
				ImGui::SetNextItemWidth(-1);
				ImGui::InputText("##AssetBrowserPath", curDirStr.GetData(), curDirStr.GetLength() + 1, dirTextFlags);

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

					if (currentVirtualRoot.GetLength() == 0)
					{
						static const char* const mounts[] = { "engine", "assets" };
						
						size_t count = sizeof(mounts) / sizeof(mounts[0]);
						for (size_t i = 0; i < count; ++i)
						{
							ImGui::PushID(static_cast<int>(i));
							DrawRootEntry(context, columnWidth, mounts[i]);
							ImGui::PopID();
						}
					}
					else
					{
						String virtualPath = RelativePathToVirtual(currentDirectory.GetRef());

						int index = 1;
						FilesystemResolver* resolver = context.filesystemResolver;
						if (resolver->ResolvePath(virtualPath.GetCStr(), context.temporaryString))
						{
							fs::path currentDirAbsolute = context.temporaryString.GetCStr();
							for (fs::directory_iterator itr(currentDirAbsolute), end; itr != end; ++itr, ++index)
							{
								ImGui::PushID(index);
								DrawEntry(context, *itr, columnWidth);
								ImGui::PopID();
							}
						}

						/*
						* Notes
						* 1. Root virtual folders engine and assets
						* 2. Root virtual folder doesn't need to get resolved to display in a list
						* 3. Once a root virtual folder is moved into, it needs to be resolved to real path in order to iterate through it's children
						* 4. currentDirectory and selectedPath need to be stored as virtual paths
						* 5. When a directory entry is shown it can use the full path from the entry
						* 6. Moving into one of the directories or selecting a file or directory requires us to convert the full patch to a virtual path
						* 7. How to find the correct virtual root? Maybe save the virtual root folder when we enter it. 
						*/
					}

					ImGui::EndTable();
				}
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

void AssetBrowserView::DrawRootEntry(EditorContext& context, float columnWidth, const char* name)
{
	ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_AllowDoubleClick;
	bool selected = selectedPath == name;
	ImGui::TableNextColumn();
	ImVec2 buttonSize(columnWidth, columnWidth + ImGui::GetFrameHeight());
	ImVec2 cursorStartPos = ImGui::GetCursorPos();
	if (ImGui::Selectable("##AssetBrowserItem", selected, selectableFlags, buttonSize))
	{
		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			currentVirtualRoot.Assign(name);
			selectedPath.Clear();
		}
		else
		{
			selectedPath.Assign(name);
		}
	}

	DrawIconAndName(cursorStartPos, editorImages->folderIcon, columnWidth, name);
}

void AssetBrowserView::DrawEntry(
	EditorContext& context,
	const std::filesystem::directory_entry& entry,
	float columnWidth)
{
	namespace fs = std::filesystem;

	bool isDir = entry.is_directory();
	bool isFile = entry.is_regular_file();

	if (isFile && entry.path().extension() == EngineConstants::MetadataExtension)
		return;

	const std::filesystem::path& entryPath = entry.path();

	ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_AllowDoubleClick;
	bool selected = entryPath == selectedPathFs;
	
	ImGui::TableNextColumn();

	std::string fileStr = entryPath.filename().u8string();
	ImVec2 textSize = ImGui::CalcTextSize(fileStr.c_str(), nullptr, false, columnWidth);
	float padding = ImGui::GetStyle().FramePadding.y * 2.0f;
	ImVec2 buttonSize(columnWidth, columnWidth + textSize.y + padding);
	ImVec2 cursorStartPos = ImGui::GetCursorPos();

	if (ImGui::Selectable("##AssetBrowserItem", selected, selectableFlags, buttonSize))
	{
		if (isFile)
		{
			bool editAsset = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
			SelectPath(context, entryPath, editAsset);
		}
		else if (isDir)
		{
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				MoveToPath(context, entryPath);
			}
			else
			{
				SelectPath(context, entryPath, false);
			}
		}
	}

	if (isFile)
	{
		if (ImGui::BeginDragDropSource())
		{
			std::string absolutePath = entryPath.u8string();
			auto result = AbsolutePathToVirtual(context, ConstStringView(absolutePath.c_str(), absolutePath.length()));
			if (result.HasValue())
			{
				if (auto asset = context.assetLibrary->FindAssetByVirtualPath(result.GetValue()))
				{
					Uid uid = asset->GetUid();
					ImGui::SetDragDropPayload(EditorConstants::AssetDragDropType, &uid, sizeof(Uid));

					ImGui::Text("%s", fileStr.c_str());
				}
			}

			ImGui::EndDragDropSource();
		}
	}

	TextureId texId = isDir ? editorImages->folderIcon : editorImages->genericFileIcon;
	DrawIconAndName(cursorStartPos, texId, columnWidth, fileStr.c_str());
}

void AssetBrowserView::DrawIconAndName(ImVec2 startPos, TextureId icon, float iconSize, const char* name)
{
	void* image = editorImages->GetImGuiTextureId(icon);
	ImVec2 uv0(0.0f, 1.0f);
	ImVec2 uv1(1.0f, 0.0f);

	ImGui::SetCursorPos(startPos);
	ImGui::Image(image, ImVec2(iconSize, iconSize), uv0, uv1);

	ImGui::TextWrapped(name);
	ImGui::Spacing();
}

void AssetBrowserView::MoveToPath(EditorContext& context, const std::filesystem::path& path)
{
	std::string absolutePath = path.u8string();
	auto result = AbsolutePathToRelative(context, ConstStringView(absolutePath.c_str(), absolutePath.length()));
	if (result.HasValue() == false)
		return;

	currentDirectory.Assign(result.GetValue());
	currentDirectory.Replace('\\', '/');

	selectedPath.Clear();
	selectedPathFs.clear();
}

void AssetBrowserView::SelectPath(EditorContext& context, const std::filesystem::path& path, bool editAsset)
{
	std::string absolutePath = path.u8string();
	auto result = AbsolutePathToRelative(context, ConstStringView(absolutePath.c_str(), absolutePath.length()));
	if (result.HasValue())
	{
		selectedPath.Assign(result.GetValue());
		selectedPathFs = path;

		if (selectedPath.GetLength() != 0)
		{
			String virtualPath = RelativePathToVirtual(selectedPath.GetRef());
			auto asset = context.assetLibrary->FindAssetByVirtualPath(virtualPath);

			if (asset != nullptr)
			{
				context.selectedAsset = asset->GetUid();

				if (editAsset)
				{
					if (asset->GetType() == AssetType::Level)
						context.requestLoadLevel = asset->GetUid();
					else
						context.editingAsset = asset->GetUid();
				}
			}
		}
	}

	context.selectedAsset = Optional<Uid>();
}

Optional<Uid> AssetBrowserView::CreateMaterial(EditorContext& context)
{
	String relativePath = currentDirectory + "/New.material";

	const AssetInfo* existingAsset = nullptr;
	int attempt = 0;
	do
	{
		if (attempt > 0)
		{
			relativePath.Resize(512);
			auto result = fmt::format_to_n(relativePath.GetData(), relativePath.GetLength(),
				"{}/New {}.material", currentDirectory.GetCStr(), attempt);

			if (result.size > relativePath.GetLength())
				break;

			relativePath.Resize(result.size);
		}

		String filePathStr = EngineConstants::VirtualMountAssets + ('/' + relativePath);
		existingAsset = context.assetLibrary->FindAssetByVirtualPath(filePathStr);
		attempt += 1;
	} while (existingAsset != nullptr && attempt < 10);

	if (existingAsset != nullptr)
	{
		KK_LOG_ERROR("Couldn't create new material. Too many files named \"New.material\".");
		return Optional<Uid>();
	}

	ConstStringView pathRef = relativePath.GetRef();
	ArrayView<const uint8_t> contentView(
		reinterpret_cast<const uint8_t*>(EditorConstants::NewAssetContentMaterial),
		std::strlen(EditorConstants::NewAssetContentMaterial));

	// Create asset
	Optional<Uid> assetUid = context.assetLibrary->CreateAsset(AssetType::Material, pathRef, contentView);

	if (assetUid.HasValue())
		KK_LOG_INFO("Material {} created", relativePath.GetCStr());
	else
		KK_LOG_ERROR("Failed to create new material asset");

	return assetUid;
}

Optional<String> AssetBrowserView::AbsolutePathToVirtual(EditorContext& context, ConstStringView absolute)
{
	auto result = AbsolutePathToRelative(context, absolute);
	if (result.HasValue())
		return RelativePathToVirtual(result.GetValue());

	return Optional<String>();
}

Optional<ConstStringView> AssetBrowserView::AbsolutePathToRelative(EditorContext& context, ConstStringView absolute)
{
	String& realRoot = pathStore;

	// Convert current virtual root to real root
	if (context.filesystemResolver->ResolvePath(currentVirtualRoot.GetCStr(), realRoot) == false)
		KK_LOG_ERROR("Current virtual root could not be resolved to real root.");
	
	intptr_t first = absolute.FindFirst(realRoot.GetRef());

	if (first == 0)
		return absolute.SubStr(realRoot.GetLength() + 1);
	else if (first < 0)
		KK_LOG_ERROR("Current real root was not found in path.");
	else
		KK_LOG_ERROR("Current real root was not at the start of the path.");

	return Optional<ConstStringView>();
}

String AssetBrowserView::RelativePathToVirtual(ConstStringView path) const
{
	String str(allocator);
	str.Append(currentVirtualRoot);
	if (path.len != 0)
	{
		str.Append('/');
		str.Append(path);
		str.Replace('\\', '/');
	}

	return str;
}

}
}
