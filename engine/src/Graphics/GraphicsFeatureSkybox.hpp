#pragma once

#include "Graphics/GraphicsFeature.hpp"

#include "Resources/MeshId.hpp"
#include "Resources/ShaderId.hpp"

namespace kokko
{

class GraphicsFeatureSkybox : public GraphicsFeature
{
public:
	GraphicsFeatureSkybox();

	virtual void Initialize(const InitializeParameters& parameters) override;
	virtual void Deinitialize(const InitializeParameters& parameters) override;
	virtual void Upload(const UploadParameters& parameters) override;
	virtual void Submit(const SubmitParameters& parameters) override;
	virtual void Render(const RenderParameters& parameters) override;

private:
	ShaderId shaderId;

	ModelId meshId;
	render::BufferId uniformBufferId;
};

}
