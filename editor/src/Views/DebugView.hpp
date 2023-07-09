#pragma once

#include "EditorWindow.hpp"

struct EditorWindowInfo;
struct EngineSettings;

class Debug;

namespace kokko
{
namespace editor
{

class DebugView : public EditorWindow
{
public:
	DebugView();

	void Initialize(Debug* debug);

	virtual void Update(EditorContext& context) override;

private:
	Debug* debug;
};

}
}
