#include "EditorProject.hpp"

#include <cstdio>

#include "ryml.hpp"

#include "Core/Core.hpp"
#include "Core/String.hpp"
#include "Core/StringView.hpp"

#include "System/Filesystem.hpp"

#include "EditorConstants.hpp"

static const char ProjectFilename[] = "project.yml";
static const char NameKey[] = "name";

namespace kokko
{
namespace editor
{

EditorProject::EditorProject(Allocator* allocator) :
	allocator(allocator),
	rootPathString(allocator),
	name(allocator)
{
}

bool EditorProject::SerializeToFile()
{
	KOKKO_PROFILE_FUNCTION();

	std::string filePath = (rootPath / ProjectFilename).u8string();

	FILE* file = std::fopen(filePath.c_str(), "w");

	if (file == nullptr)
		return false;

	ryml::Tree tree;
	ryml::NodeRef root = tree.rootref();
	root |= ryml::MAP;

	root[NameKey] << name.GetCStr();

	ryml::emit_yaml(tree, tree.root_id(), file);
	std::fclose(file);

	return true;
}

bool EditorProject::DeserializeFromFile(const std::filesystem::path& projectRootPath)
{
	KOKKO_PROFILE_FUNCTION();

	std::string filePath = (projectRootPath / ProjectFilename).u8string();

	Filesystem fs;
	String contents(allocator);
	if (fs.ReadText(filePath.c_str(), contents) == false)
		return false;

	ryml::Tree tree = ryml::parse_in_place(ryml::substr(contents.GetData(), contents.GetLength()));
	ryml::ConstNodeRef node = tree.rootref();

	if (node.is_map())
	{
		auto nameNode = node.find_child(NameKey);
		if (nameNode.valid() && nameNode.has_val())
		{
			auto nameStr = nameNode.val();
			name.Assign(ConstStringView(nameStr.str, nameStr.len));

			SetRootPath(projectRootPath);

			return true;
		}
	}

	return false;
}

const std::filesystem::path& EditorProject::GetRootPath() const
{
	return rootPath;
}

const String& EditorProject::GetRootPathString() const
{
	return rootPathString;
}

void EditorProject::SetRootPath(const std::filesystem::path& path)
{
	rootPath = path;

	std::string pathStr = rootPath.u8string();
	rootPathString.Assign(ConstStringView(pathStr.c_str(), pathStr.size()));

	assetPath = rootPath / EditorConstants::AssetDirectoryName;
}

const std::filesystem::path& EditorProject::GetAssetPath() const
{
	return assetPath;
}

const String& EditorProject::GetName() const
{
	return name;
}

void EditorProject::SetName(ConstStringView name)
{
	this->name.Assign(name);
}

}
}
