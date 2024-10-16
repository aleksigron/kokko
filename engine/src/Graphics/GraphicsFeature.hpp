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
class ModelManager;
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
		ModelManager* modelManager;
		ShaderManager* shaderManager;
	};

	struct CommonRenderParameters
	{
		PostProcessRenderer* postProcessRenderer;

		// Resource managers

		ModelManager* modelManager;
		ShaderManager* shaderManager;
		TextureManager* textureManager;

		// Component systems

		CameraSystem* cameraSystem;
		EnvironmentSystem* environmentSystem;
		LightManager* lightManager;

		// Render state

		const RenderDebugSettings& renderDebug;
		const CameraParameters& cameraParameters;

		ArrayView<const RenderViewport> viewports;
		uint32_t fullscreenViewportIndex;
		uint32_t shadowViewportsBeginIndex;
		uint32_t shadowViewportsEndIndex;

		const RenderGraphResources* renderGraphResources;
		render::FramebufferId finalTargetFramebufferId;
	};

	struct UploadParameters : public CommonRenderParameters
	{
		render::Device* renderDevice;
	};

	struct SubmitParameters : public CommonRenderParameters
	{
		GraphicsFeatureCommandList* commandList;
	};

	struct RenderParameters : public CommonRenderParameters
	{
		render::CommandEncoder* encoder;

		uint32_t renderingViewportIndex;
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
