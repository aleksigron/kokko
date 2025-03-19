#pragma once

#include "EditorWindow.hpp"

namespace kokko
{

class Debug;

namespace editor
{

class TerrainDebugView : public EditorWindow
{
public:
	TerrainDebugView();

	void Initialize(Debug* debug);
	virtual void Update(EditorContext& context) override;
};

} // namespace editor
} // namespace kokko
