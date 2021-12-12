#include "EditorProject.hpp"

#include <fstream>

#include "yaml-cpp/yaml.h"

#include "Core/Core.hpp"

#include "EditorConstants.hpp"

static const char ProjectFilename[] = "project.yml";
static const char NameKey[] = "name";

namespace kokko
{
namespace editor
{

EditorProject::EditorProject(Allocator* allocator) :
	rootPathString(allocator),
	name(allocator)
{
}

bool EditorProject::SerializeToFile()
{
	KOKKO_PROFILE_FUNCTION();

	std::ofstream outStream(rootPath / ProjectFilename);

	if (outStream.is_open() == false)
		return false;

	YAML::Emitter out(outStream);

	out << YAML::BeginMap;
	out << YAML::Key << NameKey;
	out << YAML::Value << name.GetCStr();
	out << YAML::EndMap;

	return true;
}

bool EditorProject::DeserializeFromFile(const std::filesystem::path& projectRootPath)
{
	KOKKO_PROFILE_FUNCTION();

	std::ifstream inStream(projectRootPath / ProjectFilename);

	if (inStream.is_open() == false)
		return false;

	YAML::Node node = YAML::Load(inStream);

	if (node.IsMap())
	{
		const YAML::Node nameNode = node[NameKey];
		if (nameNode.IsDefined() && nameNode.IsScalar())
		{
			const std::string& nameStr = nameNode.Scalar();
			name.Assign(StringRef(nameStr.c_str(), nameStr.size()));

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
	rootPathString.Assign(StringRef(pathStr.c_str(), pathStr.size()));

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

void EditorProject::SetName(StringRef name)
{
	this->name.Assign(name);
}

}
}
