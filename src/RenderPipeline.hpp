#pragma once

#include "SceneLayer.hpp"
#include "RenderData.hpp"
#include "RenderOrder.hpp"

class RenderPipeline
{
public:

	
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

	void BlendingEnable();
	void BlendingDisable();

	void DepthTestEnable();
	void DepthTestDisable();
	void DepthTestFunctionLess();

	void DepthWriteEnable();
	void DepthWriteDisable();

	void CullFaceEnable();
	void CullFaceDisable();
	void CullFaceFront();
	void CullFaceBack();
};
