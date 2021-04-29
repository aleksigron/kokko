#pragma once

#include <cstdint>

struct RenderCommandList;
struct RenderViewport;

class World;

class CustomRenderer
{
public:
	struct CommandParams
	{
		RenderCommandList* commandList;
		unsigned int callbackId;
		unsigned int fullscreenViewport;
		World* world;
	};

	struct RenderParams
	{
		const RenderViewport* viewport;
		unsigned int callbackId;
		uint64_t command;
		World* world;
	};

	virtual void AddRenderCommands(const CommandParams& params) {}
	virtual void RenderCustom(const RenderParams& params) {}
};
