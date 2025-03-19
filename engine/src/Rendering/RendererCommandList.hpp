#pragma once

#include "Core/Array.hpp"

#include "Resources/MaterialData.hpp"

#include "Rendering/RenderOrder.hpp"
#include "Rendering/RenderPassType.hpp"

namespace kokko
{

class Allocator;

enum class RendererCommandType
{
	Control = 0,
	Draw = 1
};

enum class RendererControlType
{
	BeginViewport,
	BeginPass
};

struct RendererCommandList
{
	RendererCommandList(Allocator* allocator) :
		commands(allocator),
		commandData(allocator)
	{
	}

	RenderOrderConfiguration renderOrder;

	Array<uint64_t> commands;
	Array<uint8_t> commandData;

	void AddControl(
		unsigned int viewport,
		RenderPassType pass,
		RendererControlType type,
		unsigned int data = 0);

	void AddControl(
		unsigned int viewport,
		RenderPassType pass,
		RendererControlType type,
		unsigned int byteCount,
		void* data);

	void AddDraw(
		unsigned int viewport,
		RenderPassType pass,
		float depth,
		MaterialId material,
		unsigned int objIndex,
		unsigned int meshPart);

	void AddDrawWithCallback(
		unsigned int viewport,
		RenderPassType pass,
		float depth,
		unsigned int callbackIndex,
		uint16_t featureObjectId = 0
	);

	void AddGraphicsFeatureWithOrder(
		unsigned int viewport,
		RenderPassType pass,
		uint32_t order,
		unsigned int featureIndex,
		uint16_t featureObjectId);

	void Sort();

	void Clear();
};

} // namespace kokko
