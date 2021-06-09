#pragma once

#include <cstdint>

#include "Rendering/CameraParameters.hpp"

struct RenderCommandList;
struct RenderViewport;

class Scene;

class CustomRenderer
{
public:
	struct CommandParams
	{
		RenderCommandList* commandList;
		unsigned int callbackId;
		unsigned int fullscreenViewport;
		Scene* scene;
	};

	struct RenderParams
	{
		const RenderViewport* viewport;
		CameraParameters cameraParams;
		unsigned int callbackId;
		uint64_t command;
		Scene* scene;
	};

	virtual void AddRenderCommands(const CommandParams& params) {}
	virtual void RenderCustom(const RenderParams& params) {}
};
