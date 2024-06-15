#pragma once

#include <cstdio>

#include "Rendering/RenderOrder.hpp"

namespace kokko
{
namespace DebugUtil
{
void FormatRenderCommand(const RenderOrderConfiguration& orderInfo, uint64_t command, char* outputBuffer)
{
	uint64_t vi = orderInfo.viewportIndex.GetValue(command);
	uint64_t vp = orderInfo.viewportPass.GetValue(command);
	uint64_t ic = orderInfo.command.GetValue(command);
	uint64_t td = orderInfo.transparentDepth.GetValue(command);
	uint64_t od = orderInfo.opaqueDepth.GetValue(command);
	uint64_t op = orderInfo.opaquePadding.GetValue(command);
	uint64_t mi = orderInfo.materialId.GetValue(command);
	uint64_t ro = orderInfo.renderObject.GetValue(command);
	uint64_t ct = orderInfo.commandType.GetValue(command);
	uint64_t cd = orderInfo.commandData.GetValue(command);

	if (ic == 1)
	{
		if (vp != 0)
			sprintf(outputBuffer, "%01llx %01llx %01llx %06llx %03llx %05llx", vi, vp, ic, td, mi, ro);
		else
			sprintf(outputBuffer, "%01llx %01llx %01llx %06llx %03llx %05llx", vi, vp, ic, od, mi, ro);
	}
	else
		sprintf(outputBuffer, "%01llx %01llx %01llx %02llx %08llx", vi, vp, ic, ct, cd);
}
} // namespace DebugUtil
} // namespace kokko
