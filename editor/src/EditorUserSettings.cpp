#include "EditorUserSettings.hpp"

#include <fstream>

#include "yaml-cpp/yaml.h"

#include "Core/Core.hpp"

static const char LastProjectKey[] = "last_opened_project";
static const char LastLevelKey[] = "last_opened_level";
static const char WindowMaximizedKey[] = "window_maximized";
static const char WindowWidthKey[] = "window_width";
static const char WindowHeightKey[] = "window_height";

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

	if (lastOpenedProject.empty() == false)
	{
		out << YAML::Key << LastProjectKey;
		out << YAML::Value << lastOpenedProject.u8string();
	}

	if (lastOpenedLevel.HasValue())
	{
		char levelUidBuf[Uid::StringLength + 1];
		lastOpenedLevel.GetValue().WriteTo(ArrayView(levelUidBuf));
		levelUidBuf[Uid::StringLength] = '\0';

		out << YAML::Key << LastLevelKey;
		out << YAML::Value << levelUidBuf;
	}

	out << YAML::Key << WindowMaximizedKey << YAML::Value << windowMaximized;
	out << YAML::Key << WindowWidthKey << YAML::Value << windowWidth;
	out << YAML::Key << WindowHeightKey << YAML::Value << windowHeight;

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

	if (node.IsMap() == false)
		return false;

	const YAML::Node projectPathNode = node[LastProjectKey];
	if (projectPathNode.IsDefined() && projectPathNode.IsScalar())
		lastOpenedProject = std::filesystem::u8path(projectPathNode.Scalar());

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

	const YAML::Node windowMaximizedNode = node[WindowMaximizedKey];
	if (windowMaximizedNode.IsDefined() && windowMaximizedNode.IsScalar())
		windowMaximized = windowMaximizedNode.as<bool>();

	const YAML::Node windowWidthNode = node[WindowWidthKey];
	if (windowWidthNode.IsDefined() && windowWidthNode.IsScalar())
		windowWidth = windowWidthNode.as<int>();

	const YAML::Node windowHeightNode = node[WindowHeightKey];
	if (windowHeightNode.IsDefined() && windowHeightNode.IsScalar())
		windowHeight = windowHeightNode.as<int>();

	return true;
}

}
}
