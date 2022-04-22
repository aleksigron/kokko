#include "Rendering/RenderCommandList.hpp"

#include "Core/Core.hpp"
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
	static const unsigned int alignment = 8;
	static const uint8_t padBuffer[alignment - 1] = { 0 };

	uint64_t c = 0;

	renderOrder.viewportIndex.AssignValue(c, viewport);
	renderOrder.viewportPass.AssignValue(c, static_cast<uint64_t>(pass));
	renderOrder.command.AssignValue(c, static_cast<uint64_t>(RenderCommandType::Control));
	renderOrder.commandOrder.AssignValue(c, order);
	renderOrder.commandType.AssignValue(c, static_cast<uint64_t>(type));

	// Always insert a multiple of 8 bytes

	size_t offset = commandData.GetCount();
	unsigned int alignedByteSize = (byteCount + alignment - 1) / alignment * alignment;
	unsigned int pad = alignedByteSize - byteCount;

	commandData.InsertBack(static_cast<uint8_t*>(data), byteCount);
	commandData.InsertBack(padBuffer, pad);

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
		uint64_t intDepth = static_cast<uint64_t>(renderOrder.maxTransparentDepth * (1.0f - depth));
		renderOrder.transparentDepth.AssignValue(c, intDepth);
	}
	else
	{
		uint64_t intDepth = static_cast<uint64_t>(renderOrder.maxOpaqueDepth * depth);
		renderOrder.opaqueDepth.AssignValue(c, intDepth);
	}

	renderOrder.materialId.AssignValue(c, material.i);
	renderOrder.renderObject.AssignValue(c, renderObjectId);

	commands.PushBack(c);
}

void RenderCommandList::AddDrawWithCallback(
	unsigned int viewport,
	RenderPass pass,
	float depth,
	unsigned int callbackIndex,
	bool isGraphicsFeature,
	uint16_t featureObjectId)
{
	depth = (depth > 1.0f ? 1.0f : (depth < 0.0f ? 0.0f : depth));

	uint64_t intpass = static_cast<uint64_t>(pass);

	uint64_t c = 0;

	renderOrder.viewportIndex.AssignValue(c, viewport);
	renderOrder.viewportPass.AssignValue(c, intpass);
	renderOrder.command.AssignValue(c, static_cast<uint64_t>(RenderCommandType::Draw));

	if ((0xfc & intpass) != 0) // Is greater than 0x03; must be transparent
	{
		uint64_t intDepth = static_cast<uint64_t>(renderOrder.maxTransparentDepth * (1.0f - depth));
		renderOrder.transparentDepth.AssignValue(c, intDepth);
	}
	else
	{
		uint64_t intDepth = static_cast<uint64_t>(renderOrder.maxOpaqueDepth * depth);
		renderOrder.opaqueDepth.AssignValue(c, intDepth);
	}

	renderOrder.materialId.AssignValue(c, RenderOrderConfiguration::CallbackMaterialId);
	renderOrder.isGraphicsFeature.AssignValue(c, static_cast<uint64_t>(isGraphicsFeature));
	renderOrder.featureIndex.AssignValue(c, callbackIndex);
	renderOrder.featureObjectId.AssignValue(c, static_cast<uint64_t>(featureObjectId));

	commands.PushBack(c);
}

void RenderCommandList::AddGraphicsFeatureWithOrder(unsigned int viewport, RenderPass pass, uint32_t order, unsigned int featureIndex, uint16_t featureObjectId)
{
	uint64_t intpass = static_cast<uint64_t>(pass);

	uint64_t c = 0;

	renderOrder.viewportIndex.AssignValue(c, viewport);
	renderOrder.viewportPass.AssignValue(c, intpass);
	renderOrder.command.AssignValue(c, static_cast<uint64_t>(RenderCommandType::Draw));

	if ((0xfc & intpass) != 0) // Is greater than 0x03; must be transparent
		renderOrder.transparentDepth.AssignValue(c, order);
	else
		renderOrder.opaqueDepth.AssignValue(c, order);

	renderOrder.materialId.AssignValue(c, RenderOrderConfiguration::CallbackMaterialId);
	renderOrder.isGraphicsFeature.AssignValue(c, static_cast<uint64_t>(1));
	renderOrder.featureIndex.AssignValue(c, featureIndex);
	renderOrder.featureObjectId.AssignValue(c, static_cast<uint64_t>(featureObjectId));

	commands.PushBack(c);
}

void RenderCommandList::Sort()
{
	KOKKO_PROFILE_FUNCTION();

	ShellSortAsc(commands.GetData(), commands.GetCount());
}

void RenderCommandList::Clear()
{
	commands.Clear();
	commandData.Clear();
}
