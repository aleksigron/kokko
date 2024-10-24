#pragma once

#include "EditorWindow.hpp"

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
