#pragma once

#include "Math/Vec2.hpp"

#include "Rendering/Framebuffer.hpp"

#include "Resources/MeshId.hpp"

#include "App/EditorCamera.hpp"

#include "Views/EditorWindow.hpp"

namespace kokko
{

class AssetInfo;
class ModelManager;
class Window;

struct CameraParameters;
struct ModelNode;
struct ModelMesh;
struct ModelMeshPart;
struct ResourceManagers;
struct SceneObjectId;

namespace editor
{

struct EditorContext;

class SceneView : public EditorWindow
{
public:
	SceneView();

	void Initialize(render::Device* renderDevice, Window* window, const ResourceManagers& resourceManagers);

	virtual void Update(EditorContext&) override;
	virtual void LateUpdate(EditorContext& context) override;

	virtual void ReleaseEngineResources() override;

	void ResizeFramebufferIfRequested();

	const render::Framebuffer& GetFramebuffer();
	Vec2i GetContentAreaSize();

	CameraParameters GetCameraParameters() const;

private:
	void ResizeFramebuffer();

	void HandleModelDragDrop(EditorContext& context, const AssetInfo* asset);

	struct ModelInfo
	{
		ModelId modelId;
		ArrayView<const ModelNode> nodes;
		ArrayView<const ModelMesh> meshes;
		ArrayView<const ModelMeshPart> meshParts;
	};

	void LoadModelNode(
		EditorContext& context,
		const ModelInfo& model,
		int16_t nodeIndex,
		SceneObjectId parent);

	int contentWidth;
	int contentHeight;

	int currentGizmoOperation;
	int currentGizmoMode;

	bool resizeRequested;
	bool windowIsFocused;
	bool windowIsHovered;

	EditorCamera editorCamera;
	render::Framebuffer framebuffer;
	ModelManager* modelManager;
};

}
}
