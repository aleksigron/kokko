#pragma once

#include "Resources/MaterialData.hpp"

#include "Rendering/RenderOrder.hpp"
#include "Rendering/RenderCommandType.hpp"

#include "Core/Array.hpp"

struct RenderCommandList
{
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

	void Sort();

	void Clear();
};
