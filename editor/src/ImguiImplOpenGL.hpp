#pragma once

#include "imgui.h"

#include "Core/Array.hpp"

#include "Rendering/RenderResourceId.hpp"

class Allocator;

namespace kokko
{

namespace render
{
class CommandEncoder;
}

class ImguiImplOpenGL
{
public:
	ImguiImplOpenGL(Allocator* allocator);
	~ImguiImplOpenGL();

	// Backend API
	void Initialize();
	void Deinitialize();

	void NewFrame();
	void RenderDrawData(render::CommandEncoder* encoder, ImDrawData* draw_data);

private:
	Array<render::VertexArrayId> vertexArrays;
	Array<render::BufferId> buffers;
};

}
