#pragma once

#include <cstdint>

#include "Core/ArrayView.hpp"

#include "Rendering/CameraParameters.hpp"
#include "Rendering/RenderDebugSettings.hpp"
#include "Rendering/RenderViewport.hpp"

namespace kokko
{

class CameraSystem;
class EnvironmentSystem;
class GraphicsFeatureCommandList;
class LightManager;
class MeshManager;
class PostProcessRenderer;
class RenderDebugSettings;
class RenderGraphResources;
class ShaderManager;
class TextureManager;

namespace render
{
class CommandEncoder;
class Device;
class Framebuffer;
}

class GraphicsFeature
{
public:
	struct InitializeParameters
	{
		render::Device* renderDevice;
		MeshManager* meshManager;
		ShaderManager* shaderManager;
	};

	struct CommonRenderParameters
	{
		PostProcessRenderer* postProcessRenderer;

		// Resource managers

		MeshManager* meshManager;
		ShaderManager* shaderManager;
		TextureManager* textureManager;

		// Component systems

		CameraSystem* cameraSystem;
		EnvironmentSystem* environmentSystem;
		LightManager* lightManager;

		// Render state

		const RenderDebugSettings& renderDebug;
		const CameraParameters& cameraParameters;
		const RenderViewport& fullscreenViewport;
		ArrayView<const RenderViewport> shadowViewports;
		const RenderGraphResources* renderGraphResources;
		render::FramebufferId finalTargetFramebufferId;
	};

	struct UploadParameters : public CommonRenderParameters
	{
		render::Device* renderDevice;
	};

	struct SubmitParameters
	{
		GraphicsFeatureCommandList& commandList;
	};

	struct RenderParameters : public CommonRenderParameters
	{
		render::CommandEncoder* encoder;

		uint16_t featureObjectId;
	};

	virtual ~GraphicsFeature() {}

	virtual void Initialize(const InitializeParameters& parameters) {}
	virtual void Deinitialize(const InitializeParameters& parameters) {}

	virtual void Upload(const UploadParameters& parameters) {}
	virtual void Submit(const SubmitParameters& parameters) = 0;
	virtual void Render(const RenderParameters& parameters) = 0;
};

} // namepace kokko
