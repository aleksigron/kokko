#pragma once

#include "EditorWindow.hpp"

namespace kokko
{

class Debug;

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
