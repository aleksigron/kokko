#include "EditorUserSettings.hpp"

#include <fstream>

#include "yaml-cpp/yaml.h"

#include "Core/Core.hpp"

static const char LastProjectKey[] = "last_opened_project";

namespace kokko
{
namespace editor
{

bool EditorUserSettings::SerializeToFile(const char* filePath)
{
	KOKKO_PROFILE_FUNCTION();

	std::ofstream outStream(filePath);

	if (outStream.is_open() == false)
		return false;

	YAML::Emitter out(outStream);

	out << YAML::BeginMap;
	out << YAML::Key << LastProjectKey;
	out << YAML::Value << lastOpenedProject.u8string();
	out << YAML::EndMap;

	return true;
}

bool EditorUserSettings::DeserializeFromFile(const char* filePath)
{
	KOKKO_PROFILE_FUNCTION();

	std::ifstream inStream(filePath);

	if (inStream.is_open() == false)
		return false;

	YAML::Node node = YAML::Load(inStream);

	if (node.IsMap())
	{
		const YAML::Node projectPathNode = node[LastProjectKey];
		if (projectPathNode.IsDefined() && projectPathNode.IsScalar())
		{
			lastOpenedProject = std::filesystem::u8path(projectPathNode.Scalar());

			return true;
		}
	}

	return false;
}

}
}
