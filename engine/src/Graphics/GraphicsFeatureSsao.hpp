#pragma once

#include "Core/Array.hpp"

#include "Math/Vec3.hpp"

#include "Graphics/GraphicsFeature.hpp"

#include "Rendering/RenderResourceId.hpp"

#include "Resources/ShaderId.hpp"

class Allocator;

namespace kokko
{

class GraphicsFeatureSsao : public GraphicsFeature
{
public:
	explicit GraphicsFeatureSsao(Allocator* allocator);

	void SetOrder(unsigned int order);

	virtual void Initialize(const InitializeParameters& parameters) override;
	virtual void Deinitialize(const InitializeParameters& parameters) override;

	virtual void Upload(const UploadParameters& parameters) override;
	virtual void Submit(const SubmitParameters& parameters) override;
	virtual void Render(const RenderParameters& parameters) override;

private:
	Allocator* allocator;
	Array<Vec3f> kernel;

	unsigned int renderOrder;

	static const size_t PassCount = 2;
	enum { PassIdx_Occlusion = 0, PassIdx_Blur = 1 };

	kokko::render::BufferId uniformBufferIds[PassCount];
	ShaderId shaderIds[PassCount];

	kokko::render::TextureId noiseTextureId;
};

}
