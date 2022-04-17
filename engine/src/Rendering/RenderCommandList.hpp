#pragma once

#include "Core/Array.hpp"

#include "Resources/MaterialData.hpp"

#include "Rendering/RenderOrder.hpp"
#include "Rendering/RenderCommandType.hpp"

class Allocator;

struct RenderCommandList
{
	RenderCommandList(Allocator* allocator) :
		commands(allocator),
		commandData(allocator)
	{
	}

	RenderOrderConfiguration renderOrder;

	Array<uint64_t> commands;
	Array<uint8_t> commandData;

	void AddControl(
		unsigned int viewport,
		RenderPass pass,
		unsigned int controlOrder,
		RenderControlType type,
		unsigned int data = 0);

	void AddControl(
		unsigned int viewport,
		RenderPass pass,
		unsigned int controlOrder,
		RenderControlType type,
		unsigned int byteCount,
		void* data);

	void AddDraw(
		unsigned int viewport,
		RenderPass pass,
		float depth,
		MaterialId material,
		unsigned int objIndex);

	void AddDrawWithCallback(
		unsigned int viewport,
		RenderPass pass,
		float depth,
		unsigned int callbackIndex,
		bool isGraphicsFeature = false,
		uint16_t featureObjectId = 0
	);

	void Sort();

	void Clear();
};
