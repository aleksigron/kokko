#pragma once

#include <cstdint>

#include "Rendering/CameraParameters.hpp"

class RenderDevice;
class MeshManager;
class ShaderManager;
class TextureManager;

namespace kokko
{

class EnvironmentSystem;

class GraphicsFeatureCommandList;

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

		CameraParameters cameraParameters;
	};

	struct SubmitParameters
	{
		GraphicsFeatureCommandList& commandList;
	};

	struct RenderParameters
	{
		RenderDevice* renderDevice;

		kokko::EnvironmentSystem* environmentSystem;
		MeshManager* meshManager;
		ShaderManager* shaderManager;
		TextureManager* textureManager;

		CameraParameters cameraParameters;
		uint16_t featureObjectId;
	};

	virtual void Initialize(const InitializeParameters& parameters) {}
	virtual void Deinitialize(const InitializeParameters& parameters) {}

	virtual void Upload(const UploadParameters& parameters) {}
	virtual void Submit(const SubmitParameters& parameters) = 0;
	virtual void Render(const RenderParameters& parameters) = 0;
};

}
