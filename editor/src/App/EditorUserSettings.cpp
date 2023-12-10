#include "EditorUserSettings.hpp"

#include <cstdio>

#include "ryml.hpp"

#include "Core/Core.hpp"
#include "Core/String.hpp"

#include "System/Filesystem.hpp"

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

	FILE* file = std::fopen(filePath, "w");

	if (file == nullptr)
		return false;

	ryml::Tree tree;
	ryml::NodeRef root = tree.rootref();
	root |= ryml::MAP;

	if (lastOpenedProject.empty() == false)
	{
		root[LastProjectKey] << lastOpenedProject.u8string().c_str();
	}

	if (lastOpenedLevel.HasValue())
	{
		char levelUidBuf[Uid::StringLength + 1];
		lastOpenedLevel.GetValue().WriteTo(ArrayView(levelUidBuf));
		levelUidBuf[Uid::StringLength] = '\0';

		root[LastLevelKey] << levelUidBuf;
	}

	root[WindowMaximizedKey] << windowMaximized;
	root[WindowWidthKey] << windowWidth;
	root[WindowHeightKey] << windowHeight;

	ryml::emit_yaml(tree, tree.root_id(), file);
	std::fclose(file);

	return true;
}

bool EditorUserSettings::DeserializeFromFile(const char* filePath, Allocator* allocator)
{
	KOKKO_PROFILE_FUNCTION();

	Filesystem fs;
	String contents(allocator);
	if (fs.ReadText(filePath, contents) == false)
		return false;

	ryml::Tree tree = ryml::parse_in_place(ryml::substr(contents.GetData(), contents.GetLength()));
	ryml::ConstNodeRef node = tree.rootref();

	if (node.is_map() == false)
		return false;

	auto projectPathNode = node.find_child(LastProjectKey);
	if (projectPathNode.valid() && projectPathNode.has_val())
		lastOpenedProject = std::filesystem::u8path(projectPathNode.val().begin(), projectPathNode.val().end());

	auto levelNode = node.find_child(LastLevelKey);
	if (levelNode.valid() && levelNode.has_val())
	{
		auto levelString = levelNode.val();
		auto uidResult = Uid::FromString(ArrayView(levelString.str, levelString.len));
		if (uidResult.HasValue())
		{
			lastOpenedLevel = uidResult.GetValue();
		}
	}

	auto windowMaximizedNode = node.find_child(WindowMaximizedKey);
	if (windowMaximizedNode.valid() && windowMaximizedNode.has_val())
		windowMaximizedNode >> windowMaximized;

	auto windowWidthNode = node.find_child(WindowWidthKey);
	if (windowWidthNode.valid() && windowWidthNode.has_val() && windowWidthNode.val().is_integer())
		windowWidthNode >> windowWidth;

	auto windowHeightNode = node.find_child(WindowHeightKey);
	if (windowHeightNode.valid() && windowHeightNode.has_val() && windowHeightNode.val().is_integer())
		windowHeightNode >> windowHeight;

	return true;
}

}
}
