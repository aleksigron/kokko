#pragma once

#include "Core/Array.hpp"

#include "Graphics/GraphicsFeature.hpp"

#include "Resources/ShaderId.hpp"

class Allocator;

namespace kokko
{

class GraphicsFeatureBloom : public GraphicsFeature
{
public:
	explicit GraphicsFeatureBloom(Allocator* allocator);

	void SetOrder(unsigned int order);

	virtual void Initialize(const InitializeParameters& parameters) override;
	virtual void Deinitialize(const InitializeParameters& parameters) override;

	virtual void Submit(const SubmitParameters& parameters) override;
	virtual void Render(const RenderParameters& parameters) override;

private:
	void CreateKernel(int kernelExtent);

	Allocator* allocator;

	Array<float> blurKernel;

	unsigned int renderOrder;

	unsigned char* uniformStagingBuffer;

	ShaderId extractShaderId;
	ShaderId downsampleShaderId;
	ShaderId upsampleShaderId;
	ShaderId applyShaderId;

	unsigned int uniformBlockStride;

	unsigned int uniformBufferId;
	unsigned int linearSamplerId;
};

}
