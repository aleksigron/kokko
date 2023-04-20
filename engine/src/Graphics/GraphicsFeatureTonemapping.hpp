#pragma once

#include "Graphics/GraphicsFeature.hpp"

#include "Rendering/RenderResourceId.hpp"

#include "Resources/ShaderId.hpp"

namespace kokko
{

class GraphicsFeatureTonemapping : public GraphicsFeature
{
public:
	GraphicsFeatureTonemapping();

	void SetOrder(unsigned int order);

	virtual void Initialize(const InitializeParameters& parameters) override;
	virtual void Deinitialize(const InitializeParameters& parameters) override;

	virtual void Upload(const UploadParameters& parameters) override;

	virtual void Submit(const SubmitParameters& parameters) override;
	virtual void Render(const RenderParameters& parameters) override;

private:
	unsigned int renderOrder;

	ShaderId shaderId;
	kokko::render::BufferId uniformBufferId;
};

}
