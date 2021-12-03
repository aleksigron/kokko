#pragma once

#include "EditorWindow.hpp"

class MaterialManager;

namespace kokko
{
namespace editor
{

class AssetView : public EditorWindow
{
public:
	AssetView();

	void Initialize(MaterialManager* materialManager);

	virtual void Update(EditorContext& context) override;

private:
	MaterialManager* materialManager;
};

}
}
