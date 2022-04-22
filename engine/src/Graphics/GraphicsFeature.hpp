#pragma once

#include <cstdint>

#include "Core/ArrayView.hpp"

#include "Rendering/CameraParameters.hpp"
#include "Rendering/RenderViewport.hpp"

class Framebuffer;
class LightManager;
class MeshManager;
class PostProcessRenderer;
class RenderDevice;
class ShaderManager;
class TextureManager;

struct RenderViewport;

namespace kokko
{

class EnvironmentSystem;
class GraphicsFeatureCommandList;
class RenderGraphResources;

class GraphicsFeature
{
public:
	struct InitializeParameters
	{
		RenderDevice* renderDevice;
		MeshManager* meshManager;
		ShaderManager* shaderManager;
	};

	struct UploadParameters
	{
		RenderDevice* renderDevice;

		// Render state

		const CameraParameters& cameraParameters;
		const RenderViewport& fullscreenViewport;
	};

	struct SubmitParameters
	{
		GraphicsFeatureCommandList& commandList;
	};

	struct RenderParameters
	{
		RenderDevice* renderDevice;
		PostProcessRenderer* postProcessRenderer;

		// Resource managers

		MeshManager* meshManager;
		ShaderManager* shaderManager;
		TextureManager* textureManager;

		// Component systems

		kokko::EnvironmentSystem* environmentSystem;
		LightManager* lightManager;
		
		// Render state

		const CameraParameters& cameraParameters;
		const RenderViewport& fullscreenViewport;
		ArrayView<const RenderViewport> shadowViewports;
		const RenderGraphResources* renderGraphResources;

		// Feature parameters

		uint16_t featureObjectId;
	};

	virtual void Initialize(const InitializeParameters& parameters) {}
	virtual void Deinitialize(const InitializeParameters& parameters) {}

	virtual void Upload(const UploadParameters& parameters) {}
	virtual void Submit(const SubmitParameters& parameters) = 0;
	virtual void Render(const RenderParameters& parameters) = 0;
};

}
