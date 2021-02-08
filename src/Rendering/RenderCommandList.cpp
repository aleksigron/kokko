#include "Rendering/RenderCommandList.hpp"

#include "Core/Sort.hpp"

void RenderCommandList::AddControl(
	unsigned int viewport,
	RenderPass pass,
	unsigned int order,
	RenderControlType type,
	unsigned int data)
{
	uint64_t c = 0;

	renderOrder.viewportIndex.AssignValue(c, viewport);
	renderOrder.viewportPass.AssignValue(c, static_cast<uint64_t>(pass));
	renderOrder.command.AssignValue(c, static_cast<uint64_t>(RenderCommandType::Control));
	renderOrder.commandOrder.AssignValue(c, order);
	renderOrder.commandType.AssignValue(c, static_cast<uint64_t>(type));
	renderOrder.commandData.AssignValue(c, data);

	commands.PushBack(c);
}

void RenderCommandList::AddControl(
	unsigned int viewport,
	RenderPass pass,
	unsigned int order,
	RenderControlType type,
	unsigned int byteCount,
	void* data)
{
	uint64_t c = 0;

	renderOrder.viewportIndex.AssignValue(c, viewport);
	renderOrder.viewportPass.AssignValue(c, static_cast<uint64_t>(pass));
	renderOrder.command.AssignValue(c, static_cast<uint64_t>(RenderCommandType::Control));
	renderOrder.commandOrder.AssignValue(c, order);
	renderOrder.commandType.AssignValue(c, static_cast<uint64_t>(type));

	unsigned int offset = commandData.GetCount();
	commandData.InsertBack(static_cast<uint8_t*>(data), byteCount);

	// TODO: Align to 4-byte boundary

	renderOrder.commandData.AssignValue(c, offset);

	commands.PushBack(c);
}

void RenderCommandList::AddDraw(
	unsigned int viewport,
	RenderPass pass,
	float depth,
	MaterialId material,
	unsigned int renderObjectId)
{
	depth = (depth > 1.0f ? 1.0f : (depth < 0.0f ? 0.0f : depth));

	uint64_t intpass = static_cast<uint64_t>(pass);

	uint64_t c = 0;

	renderOrder.viewportIndex.AssignValue(c, viewport);
	renderOrder.viewportPass.AssignValue(c, intpass);
	renderOrder.command.AssignValue(c, static_cast<uint64_t>(RenderCommandType::Draw));

	if ((0xfc & intpass) != 0) // Is greater than 0x03; must be transparent
	{
		uint64_t intDepth(renderOrder.maxTransparentDepth * (1.0f - depth));
		renderOrder.transparentDepth.AssignValue(c, intDepth);
	}
	else
	{
		uint64_t intDepth(renderOrder.maxOpaqueDepth * depth);
		renderOrder.opaqueDepth.AssignValue(c, intDepth);
	}

	renderOrder.materialId.AssignValue(c, material.i);
	renderOrder.renderObject.AssignValue(c, renderObjectId);

	commands.PushBack(c);
}

void RenderCommandList::AddDrawWithCallback(unsigned int viewport, RenderPass pass, float depth, unsigned int callbackIndex)
{
	depth = (depth > 1.0f ? 1.0f : (depth < 0.0f ? 0.0f : depth));

	uint64_t intpass = static_cast<uint64_t>(pass);

	uint64_t c = 0;

	renderOrder.viewportIndex.AssignValue(c, viewport);
	renderOrder.viewportPass.AssignValue(c, intpass);
	renderOrder.command.AssignValue(c, static_cast<uint64_t>(RenderCommandType::Draw));

	if ((0xfc & intpass) != 0) // Is greater than 0x03; must be transparent
	{
		uint64_t intDepth(renderOrder.maxTransparentDepth * (1.0f - depth));
		renderOrder.transparentDepth.AssignValue(c, intDepth);
	}
	else
	{
		uint64_t intDepth(renderOrder.maxOpaqueDepth * depth);
		renderOrder.opaqueDepth.AssignValue(c, intDepth);
	}

	renderOrder.materialId.AssignValue(c, RenderOrderConfiguration::CallbackMaterialId);
	renderOrder.renderObject.AssignValue(c, callbackIndex);

	commands.PushBack(c);
}

void RenderCommandList::Sort()
{
	ShellSortAsc(commands.GetData(), commands.GetCount());
}

void RenderCommandList::Clear()
{
	commands.Clear();
	commandData.Clear();
}
