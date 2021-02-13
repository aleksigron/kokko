#pragma once

#include <cstdint>

struct RenderCommandList;
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
		unsigned int callbackId;
		uint64_t command;
		Scene* scene;
	};

	virtual void AddRenderCommands(const CommandParams& params) {}
	virtual void RenderCustom(const RenderParams& params) {}
};
