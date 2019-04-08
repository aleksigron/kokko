#pragma once

#include "SceneLayer.hpp"
#include "TransparencyType.hpp"
#include "MaterialData.hpp"

#include "RenderOrder.hpp"
#include "RenderCommandType.hpp"

#include "Array.hpp"

struct RenderCommandSlot
{
	SceneLayer layer;
	unsigned char pass;

	RenderCommandSlot()
	{
	}

	RenderCommandSlot(SceneLayer layer, TransparencyType transparency) :
		layer(layer),
		pass(static_cast<unsigned char>(transparency))
	{
	}

	RenderCommandSlot(SceneLayer layer, RenderPass pass) :
		layer(layer),
		pass(static_cast<unsigned char>(pass))
	{
	}

};

struct RenderCommandList
{
	RenderOrderConfiguration renderOrder;

	Array<uint64_t> commands;
	Array<uint8_t> commandData;

	void AddControl(
		RenderCommandSlot slot,
		unsigned int controlOrder,
		RenderControlType type,
		unsigned int data = 0);

	void AddControl(
		RenderCommandSlot slot,
		unsigned int controlOrder,
		RenderControlType type,
		unsigned int byteCount,
		void* data);

	void AddDraw(
		RenderCommandSlot slot,
		float depth,
		MaterialId material,
		unsigned int objIndex);

	void Sort();

	void Clear();
};
