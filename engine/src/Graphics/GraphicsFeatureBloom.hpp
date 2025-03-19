#pragma once

#include "Core/Array.hpp"

#include "Graphics/GraphicsFeature.hpp"

#include "Rendering/PostProcessRenderer.hpp"
#include "Rendering/RenderResourceId.hpp"
#include "Rendering/RenderTargetContainer.hpp"

#include "Resources/ShaderId.hpp"

namespace kokko
{

class Allocator;

class GraphicsFeatureBloom : public GraphicsFeature
{
public:
	explicit GraphicsFeatureBloom(Allocator* allocator);

	void SetOrder(unsigned int order);

	virtual void Initialize(const InitializeParameters& parameters) override;
	virtual void Deinitialize(const InitializeParameters& parameters) override;

	virtual void Upload(const UploadParameters& parameters) override;

	virtual void Submit(const SubmitParameters& parameters) override;
	virtual void Render(const RenderParameters& parameters) override;

private:
	void CreateKernel(int kernelExtent);

	Allocator* allocator;

	Array<float> blurKernel;
	Array<PostProcessRenderPass> renderPasses;
	Array<RenderTarget> renderTargets;

	Array<uint8_t> uniformStagingBuffer;

	unsigned int renderOrder;

	ShaderId extractShaderId;
	ShaderId downsampleShaderId;
	ShaderId upsampleShaderId;
	ShaderId applyShaderId;

	unsigned int uniformBlockStride;

	render::BufferId uniformBufferId;
	render::SamplerId linearSamplerId;
};

} // namespace kokko
