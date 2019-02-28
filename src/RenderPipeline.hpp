#pragma once

#include "SceneLayer.hpp"
#include "RenderData.hpp"
#include "RenderOrder.hpp"

struct Color;

class RenderPipeline
{
private:
	RenderOrderConfiguration orderConfig;

	void InitializeRenderOrder();

public:
	RenderPipeline();
	~RenderPipeline();

	bool ParseControlCommand(uint64_t orderKey);
	uint64_t CreateControlCommand(SceneLayer layer, TransparencyType transparency,
								  RenderOrder::ControlCommandType command);

	uint64_t CreateDrawCommand(SceneLayer layer, TransparencyType transparency,
							   float depth, unsigned int materialId);

	static void ClearColorAndDepth(const Color& color);

	static void BlendingEnable();
	static void BlendingDisable();

	static void DepthTestEnable();
	static void DepthTestDisable();
	static void DepthTestFunctionLess();

	static void DepthWriteEnable();
	static void DepthWriteDisable();

	static void CullFaceEnable();
	static void CullFaceDisable();
	static void CullFaceFront();
	static void CullFaceBack();
};
