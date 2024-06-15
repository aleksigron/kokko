#include "Rendering/RendererCommandList.hpp"

#include "Core/Core.hpp"
#include "Core/Sort.hpp"

namespace kokko
{

void RendererCommandList::AddControl(
	unsigned int viewport,
	RenderPassType pass,
	RendererControlType type,
	unsigned int data)
{
	uint64_t c = 0;

	renderOrder.viewportIndex.AssignValue(c, viewport);
	renderOrder.viewportPass.AssignValue(c, static_cast<uint64_t>(pass));
	renderOrder.command.AssignValue(c, static_cast<uint64_t>(RendererCommandType::Control));
	renderOrder.commandType.AssignValue(c, static_cast<uint64_t>(type));
	renderOrder.commandData.AssignValue(c, data);

	commands.PushBack(c);
}

void RendererCommandList::AddControl(
	unsigned int viewport,
	RenderPassType pass,
	RendererControlType type,
	unsigned int byteCount,
	void* data)
{
	static const unsigned int alignment = 8;
	static const uint8_t padBuffer[alignment - 1] = { 0 };

	uint64_t c = 0;

	renderOrder.viewportIndex.AssignValue(c, viewport);
	renderOrder.viewportPass.AssignValue(c, static_cast<uint64_t>(pass));
	renderOrder.command.AssignValue(c, static_cast<uint64_t>(RendererCommandType::Control));
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

void RendererCommandList::AddDraw(
	unsigned int viewport,
	RenderPassType pass,
	float depth,
	MaterialId material,
	unsigned int renderObjectId,
	unsigned int meshPart)
{
	depth = (depth > 1.0f ? 1.0f : (depth < 0.0f ? 0.0f : depth));

	uint64_t intpass = static_cast<uint64_t>(pass);

	uint64_t c = 0;

	renderOrder.viewportIndex.AssignValue(c, viewport);
	renderOrder.viewportPass.AssignValue(c, intpass);
	renderOrder.command.AssignValue(c, static_cast<uint64_t>(RendererCommandType::Draw));

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
	renderOrder.meshPart.AssignValue(c, meshPart);

	commands.PushBack(c);
}

void RendererCommandList::AddDrawWithCallback(
	unsigned int viewport,
	RenderPassType pass,
	float depth,
	unsigned int callbackIndex,
	uint16_t featureObjectId)
{
	depth = (depth > 1.0f ? 1.0f : (depth < 0.0f ? 0.0f : depth));

	uint64_t intpass = static_cast<uint64_t>(pass);

	uint64_t c = 0;

	renderOrder.viewportIndex.AssignValue(c, viewport);
	renderOrder.viewportPass.AssignValue(c, intpass);
	renderOrder.command.AssignValue(c, static_cast<uint64_t>(RendererCommandType::Draw));

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
	renderOrder.featureIndex.AssignValue(c, callbackIndex);
	renderOrder.featureObjectId.AssignValue(c, static_cast<uint64_t>(featureObjectId));

	commands.PushBack(c);
}

void RendererCommandList::AddGraphicsFeatureWithOrder(unsigned int viewport, RenderPassType pass, uint32_t order, unsigned int featureIndex, uint16_t featureObjectId)
{
	uint64_t intpass = static_cast<uint64_t>(pass);

	uint64_t c = 0;

	renderOrder.viewportIndex.AssignValue(c, viewport);
	renderOrder.viewportPass.AssignValue(c, intpass);
	renderOrder.command.AssignValue(c, static_cast<uint64_t>(RendererCommandType::Draw));

	if ((0xfc & intpass) != 0) // Is greater than 0x03; must be transparent
		renderOrder.transparentDepth.AssignValue(c, order);
	else
		renderOrder.opaqueDepth.AssignValue(c, order);

	renderOrder.materialId.AssignValue(c, RenderOrderConfiguration::CallbackMaterialId);
	renderOrder.featureIndex.AssignValue(c, featureIndex);
	renderOrder.featureObjectId.AssignValue(c, static_cast<uint64_t>(featureObjectId));

	commands.PushBack(c);
}

void RendererCommandList::Sort()
{
	KOKKO_PROFILE_FUNCTION();

	ShellSortAsc(commands.GetData(), commands.GetCount());
}

void RendererCommandList::Clear()
{
	commands.Clear();
	commandData.Clear();
}

} // namespace kokko
