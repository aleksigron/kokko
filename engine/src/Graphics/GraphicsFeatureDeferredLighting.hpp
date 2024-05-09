#pragma once

#include "Core/Array.hpp"

#include "Graphics/GraphicsFeature.hpp"

#include "Rendering/Light.hpp"
#include "Rendering/RenderResourceId.hpp"

#include "Resources/MeshId.hpp"
#include "Resources/ShaderId.hpp"

class Allocator;

namespace kokko
{

class GraphicsFeatureDeferredLighting : public GraphicsFeature
{
public:
	explicit GraphicsFeatureDeferredLighting(Allocator* allocator);

	void SetOrder(unsigned int order);

	virtual void Initialize(const InitializeParameters& parameters) override;
	virtual void Deinitialize(const InitializeParameters& parameters) override;

	virtual void Upload(const UploadParameters& parameters) override;

	virtual void Submit(const SubmitParameters& parameters) override;
	virtual void Render(const RenderParameters& parameters) override;

private:
	Array<LightId> lightResultArray;

	ShaderId shaderId;

	ModelId meshId;

	unsigned int renderOrder;

	kokko::render::BufferId uniformBufferId;
	kokko::render::FramebufferId brdfLutFramebufferId;
	kokko::render::TextureId brdfLutTextureId;

	enum
	{
		Sampler_DepthCompare,
		Sampler_ClampLinear,
		Sampler_Mipmap,
		Sampler_COUNT
	};
	kokko::render::SamplerId samplers[Sampler_COUNT];
};

}
