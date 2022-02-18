#include "EditorUserSettings.hpp"

#include <fstream>

#include "yaml-cpp/yaml.h"

#include "Core/Core.hpp"

static const char LastProjectKey[] = "last_opened_project";
static const char LastLevelKey[] = "last_opened_level";

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

	if (lastOpenedLevel.HasValue())
	{
		char levelUidBuf[Uid::StringLength + 1];
		lastOpenedLevel.GetValue().WriteTo(ArrayView(levelUidBuf));
		levelUidBuf[Uid::StringLength] = '\0';

		out << YAML::Key << LastLevelKey;
		out << YAML::Value << levelUidBuf;
	}

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
		const YAML::Node levelNode = node[LastLevelKey];
		if (levelNode.IsDefined() && levelNode.IsScalar())
		{
			const std::string& levelString = levelNode.Scalar();
			auto uidResult = Uid::FromString(ArrayView(levelString.c_str(), levelString.length()));
			if (uidResult.HasValue())
			{
				lastOpenedLevel = uidResult.GetValue();
			}
		}

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
