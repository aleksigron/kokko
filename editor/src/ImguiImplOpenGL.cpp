#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "ImguiImplOpenGL.hpp"

#include "Rendering/RenderCommandEncoder.hpp"

#include <cstdio>
#include <cstdint>

#include "System/IncludeOpenGL.hpp"

// OpenGL Data
static char         g_GlslVersionString[32] = "#version 450 core\n";
static GLuint       g_FontTexture = 0;
static GLuint       g_ShaderHandle = 0;
static GLint        g_UniformLocationTex = 0;
static GLuint       g_AttribLocationVtxPos = 0, g_AttribLocationVtxUV = 0, g_AttribLocationVtxColor = 0;

// Forward Declarations
static void ImGui_ImplOpenGL3_InitPlatformInterface();
static void ImGui_ImplOpenGL3_ShutdownPlatformInterface();

namespace kokko
{

void ImGui_ImplOpenGL3_DestroyFontsTexture();
bool ImGui_ImplOpenGL3_CreateDeviceObjects();

ImguiImplOpenGL::ImguiImplOpenGL(Allocator* allocator) :
	vertexArrays(allocator),
	buffers(allocator)
{
}

ImguiImplOpenGL::~ImguiImplOpenGL()
{
}

// Functions
void ImguiImplOpenGL::Initialize()
{
	// Setup backend capabilities flags
	ImGuiIO& io = ImGui::GetIO();
	io.BackendRendererName = "imgui_impl_opengl45";
	io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
	io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;  // We can create multi-viewports on the Renderer side (optional)

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		ImGui_ImplOpenGL3_InitPlatformInterface();
}

void ImguiImplOpenGL::Deinitialize()
{
	ImGui_ImplOpenGL3_ShutdownPlatformInterface();

	if (g_ShaderHandle)
	{
		glDeleteProgram(g_ShaderHandle);
		g_ShaderHandle = 0;
	}

	ImGui_ImplOpenGL3_DestroyFontsTexture();

	if (vertexArrays.GetCount() > 0)
	{
		glDeleteVertexArrays(vertexArrays.GetCount(), &vertexArrays[0].i);
		vertexArrays.Clear();
	}

	if (buffers.GetCount() > 0)
	{
		glDeleteBuffers(buffers.GetCount(), &buffers[0].i);
		buffers.Clear();
	}
}

void ImguiImplOpenGL::NewFrame()
{
	if (!g_ShaderHandle)
		ImGui_ImplOpenGL3_CreateDeviceObjects();
}

void ImguiImplOpenGL::RenderDrawData(render::CommandEncoder* encoder, ImDrawData* draw_data)
{
	// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
	int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
	int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
	if (fb_width <= 0 || fb_height <= 0)
		return;

	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 16, "ImGuiImpl_Upload");

	glDeleteVertexArrays((int)vertexArrays.GetCount(), &vertexArrays[0].i);
	glDeleteBuffers((int)buffers.GetCount(), &buffers[0].i);

	int vertexArrayCount = draw_data->CmdListsCount;
	int bufferCount = draw_data->CmdListsCount * 2 + 1;
	int uniformBufferIndex = bufferCount - 1;

	vertexArrays.Resize(vertexArrayCount);
	buffers.Resize(bufferCount);

	glCreateVertexArrays(vertexArrayCount, &vertexArrays[0].i);
	glCreateBuffers(bufferCount, &buffers[0].i);

	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];

		uint32_t va = vertexArrays[n].i;
		uint32_t vb = buffers[n * 2 + 0].i;
		uint32_t ib = buffers[n * 2 + 1].i;

		// Upload vertex/index buffers
		glNamedBufferStorage(
			vb, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), cmd_list->VtxBuffer.Data, 0);
		glNamedBufferStorage(
			ib, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), cmd_list->IdxBuffer.Data, 0);

		// Set up vertex array

		glVertexArrayVertexBuffer(va, 0, buffers[n * 2 + 0].i, 0, sizeof(ImDrawVert));
		glVertexArrayElementBuffer(va, buffers[n * 2 + 1].i);

		glEnableVertexArrayAttrib(va, g_AttribLocationVtxPos);
		glEnableVertexArrayAttrib(va, g_AttribLocationVtxUV);
		glEnableVertexArrayAttrib(va, g_AttribLocationVtxColor);

		glVertexArrayAttribBinding(va, g_AttribLocationVtxPos, 0);
		glVertexArrayAttribBinding(va, g_AttribLocationVtxUV, 0);
		glVertexArrayAttribBinding(va, g_AttribLocationVtxColor, 0);

		glVertexArrayAttribFormat(
			va, g_AttribLocationVtxPos, 2, GL_FLOAT, GL_FALSE, IM_OFFSETOF(ImDrawVert, pos));
		glVertexArrayAttribFormat(
			va, g_AttribLocationVtxUV, 2, GL_FLOAT, GL_FALSE, IM_OFFSETOF(ImDrawVert, uv));
		glVertexArrayAttribFormat(
			va, g_AttribLocationVtxColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, IM_OFFSETOF(ImDrawVert, col));
	}

	{
		// Uniform buffer

		float L = draw_data->DisplayPos.x;
		float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
		float T = draw_data->DisplayPos.y;
		float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;

		const float ortho_projection[4][4] =
		{
			{ 2.0f / (R - L),   0.0f,         0.0f,   0.0f },
			{ 0.0f,         2.0f / (T - B),   0.0f,   0.0f },
			{ 0.0f,         0.0f,        -1.0f,   0.0f },
			{ (R + L) / (L - R),  (T + B) / (B - T),  0.0f,   1.0f },
		};

		uint32_t ub = buffers[uniformBufferIndex].i;
		glNamedBufferStorage(ub, sizeof(ortho_projection), &ortho_projection[0][0], 0);
	}

	glPopDebugGroup();

	auto scope = encoder->CreateDebugScope(0, kokko::ConstStringView("ImGuiImpl_Render"));

	// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
	encoder->BlendingEnable();
	encoder->SetBlendEquation(0, RenderBlendEquation::Add);
	encoder->SetBlendFunctionSeparate(
		0, RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha,
		RenderBlendFactor::One, RenderBlendFactor::OneMinusSrcAlpha);
	encoder->SetCullFace(RenderCullFace::None);
	encoder->DepthTestDisable();
	encoder->StencilTestDisable();
	encoder->ScissorTestEnable();

	encoder->SetViewport(0, 0, fb_width, fb_height);
	encoder->UseShaderProgram(RenderShaderId(g_ShaderHandle));
	encoder->BindBufferBase(RenderBufferTarget::UniformBuffer, 0, buffers[uniformBufferIndex]);

	encoder->BindSampler(0, RenderSamplerId());

	const auto indexType = sizeof(ImDrawIdx) == 2 ? RenderIndexType::UnsignedShort : RenderIndexType::UnsignedInt;

	// Will project scissor/clipping rectangles into framebuffer space
	ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
	ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

	// Render command lists
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];

		encoder->BindVertexArray(vertexArrays[n]);

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

			// Project scissor/clipping rectangles into framebuffer space
			ImVec4 clip_rect;
			clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
			clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
			clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
			clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

			if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
			{
				encoder->SetScissorRectangle(
					(int)clip_rect.x,
					(int)(fb_height - clip_rect.w),
					(int)(clip_rect.z - clip_rect.x),
					(int)(clip_rect.w - clip_rect.y));

				auto texId = RenderTextureId((GLuint)(intptr_t)pcmd->GetTexID());
				encoder->BindTextureToShader(g_UniformLocationTex, 0, texId);

				encoder->DrawIndexed(RenderPrimitiveMode::Triangles, indexType, pcmd->ElemCount,
					pcmd->IdxOffset * sizeof(ImDrawIdx), pcmd->VtxOffset);
			}
		}
	}
}

bool ImGui_ImplOpenGL3_CreateFontsTexture()
{
	// Build texture atlas
	ImGuiIO& io = ImGui::GetIO();
	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bit (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

#ifdef GL_UNPACK_ROW_LENGTH
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif

	// Upload texture to graphics system
	glCreateTextures(GL_TEXTURE_2D, 1, &g_FontTexture);
	glTextureParameteri(g_FontTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(g_FontTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureStorage2D(g_FontTexture, 1, GL_RGBA8, width, height);
	glTextureSubImage2D(g_FontTexture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	// Store our identifier
	io.Fonts->SetTexID((ImTextureID)(intptr_t)g_FontTexture);

	return true;
}

void ImGui_ImplOpenGL3_DestroyFontsTexture()
{
	if (g_FontTexture)
	{
		ImGuiIO& io = ImGui::GetIO();
		glDeleteTextures(1, &g_FontTexture);
		io.Fonts->SetTexID(0);
		g_FontTexture = 0;
	}
}

// If you get an error please report on github. You may try different GL context version or GLSL version. See GL<>GLSL version table at the top of this file.
static bool CheckShader(GLuint handle, const char* desc)
{
	GLint status = 0, log_length = 0;
	glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
	glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_length);
	if ((GLboolean)status == GL_FALSE)
		fprintf(stderr, "ERROR: ImGui_ImplOpenGL3_CreateDeviceObjects: failed to compile %s!\n", desc);
	if (log_length > 1)
	{
		ImVector<char> buf;
		buf.resize((int)(log_length + 1));
		glGetShaderInfoLog(handle, log_length, NULL, (GLchar*)buf.begin());
		fprintf(stderr, "%s\n", buf.begin());
	}
	return (GLboolean)status == GL_TRUE;
}

// If you get an error please report on GitHub. You may try different GL context version or GLSL version.
static bool CheckProgram(GLuint handle, const char* desc)
{
	GLint status = 0, log_length = 0;
	glGetProgramiv(handle, GL_LINK_STATUS, &status);
	glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_length);
	if ((GLboolean)status == GL_FALSE)
		fprintf(stderr, "ERROR: ImGui_ImplOpenGL3_CreateDeviceObjects: failed to link %s! (with GLSL '%s')\n", desc, g_GlslVersionString);
	if (log_length > 1)
	{
		ImVector<char> buf;
		buf.resize((int)(log_length + 1));
		glGetProgramInfoLog(handle, log_length, NULL, (GLchar*)buf.begin());
		fprintf(stderr, "%s\n", buf.begin());
	}
	return (GLboolean)status == GL_TRUE;
}

bool ImGui_ImplOpenGL3_CreateDeviceObjects()
{
	const GLchar* vertex_shader =
		"layout (location = 0) in vec2 Position;\n"
		"layout (location = 1) in vec2 UV;\n"
		"layout (location = 2) in vec4 Color;\n"
		"layout (std140, binding = 0) uniform Matrices { mat4 ProjMtx; };\n"
		"out vec2 Frag_UV;\n"
		"out vec4 Frag_Color;\n"
		"void main()\n"
		"{\n"
		"    Frag_UV = UV;\n"
		"    Frag_Color = Color;\n"
		"    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
		"}\n";

	const GLchar* fragment_shader =
		"in vec2 Frag_UV;\n"
		"in vec4 Frag_Color;\n"
		"uniform sampler2D Texture;\n"
		"layout (location = 0) out vec4 Out_Color;\n"
		"void main()\n"
		"{\n"
		"    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
		"}\n";

	// Create shaders
	const GLchar* vertex_shader_with_version[2] = { g_GlslVersionString, vertex_shader };
	GLuint vert_handle = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vert_handle, 2, vertex_shader_with_version, NULL);
	glCompileShader(vert_handle);
	CheckShader(vert_handle, "vertex shader");

	const GLchar* fragment_shader_with_version[2] = { g_GlslVersionString, fragment_shader };
	GLuint frag_handle = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag_handle, 2, fragment_shader_with_version, NULL);
	glCompileShader(frag_handle);
	CheckShader(frag_handle, "fragment shader");

	// Link
	g_ShaderHandle = glCreateProgram();
	glAttachShader(g_ShaderHandle, vert_handle);
	glAttachShader(g_ShaderHandle, frag_handle);
	glLinkProgram(g_ShaderHandle);
	CheckProgram(g_ShaderHandle, "shader program");

	glDetachShader(g_ShaderHandle, vert_handle);
	glDetachShader(g_ShaderHandle, frag_handle);
	glDeleteShader(vert_handle);
	glDeleteShader(frag_handle);

	g_UniformLocationTex = glGetUniformLocation(g_ShaderHandle, "Texture");
	g_AttribLocationVtxPos = (GLuint)glGetAttribLocation(g_ShaderHandle, "Position");
	g_AttribLocationVtxUV = (GLuint)glGetAttribLocation(g_ShaderHandle, "UV");
	g_AttribLocationVtxColor = (GLuint)glGetAttribLocation(g_ShaderHandle, "Color");

	ImGui_ImplOpenGL3_CreateFontsTexture();

	return true;
}

} // namespace kokko

//--------------------------------------------------------------------------------------------------------
// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
// This is an _advanced_ and _optional_ feature, allowing the backend to create and handle multiple viewports simultaneously.
// If you are new to dear imgui or creating a new binding for dear imgui, it is recommended that you completely ignore this section first..
//--------------------------------------------------------------------------------------------------------

static void ImGui_ImplOpenGL3_RenderWindow(ImGuiViewport* viewport, void*)
{
	if (!(viewport->Flags & ImGuiViewportFlags_NoRendererClear))
	{
		ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	// TODO: Re-enable
	//ImGui_ImplOpenGL3_RenderDrawData(viewport->DrawData);
}

static void ImGui_ImplOpenGL3_InitPlatformInterface()
{
	ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
	platform_io.Renderer_RenderWindow = ImGui_ImplOpenGL3_RenderWindow;
}

static void ImGui_ImplOpenGL3_ShutdownPlatformInterface()
{
	ImGui::DestroyPlatformWindows();
}
