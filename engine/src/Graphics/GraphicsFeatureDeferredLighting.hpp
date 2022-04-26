#pragma once

#include "Core/Array.hpp"

#include "Graphics/GraphicsFeature.hpp"

#include "Rendering/Light.hpp"

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

	virtual void Submit(const SubmitParameters& parameters) override;
	virtual void Render(const RenderParameters& parameters) override;

private:
	Array<LightId> lightResultArray;

	ShaderId shaderId;

	MeshId meshId;

	unsigned int renderOrder;

	unsigned int uniformBufferId;

	unsigned int brdfLutTextureId;
};

}