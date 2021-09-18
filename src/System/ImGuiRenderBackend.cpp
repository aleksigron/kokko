#include "System/ImGuiRenderBackend.hpp"

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <cstdint>
#include <cstdio>

#include "System/IncludeOpenGL.hpp"

#include "imgui.h"

ImGuiRenderBackend::ImGuiRenderBackend() :
	fontTexture(0),
	shaderProgramId(0),
	vertexShaderId(0),
	fragmentShaderId(0),
	uniformLocationTex(-1),
	uniformLocationProj(-1),
	attribLocationPos(0),
	attribLocationUV(0),
	attribLocationCol(0),
	vertexBufferId(0),
	indexBufferId(0)
{
}

ImGuiRenderBackend::~ImGuiRenderBackend()
{
	Deinitialize();
}

bool ImGuiRenderBackend::Initialize()
{
	// Setup backend capabilities flags
	ImGuiIO& io = ImGui::GetIO();
	io.BackendRendererName = "ImGuiRenderBackend";

	// We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
	io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

	return true;
}

void ImGuiRenderBackend::Deinitialize()
{
	if (vertexBufferId)
	{
		glDeleteBuffers(1, &vertexBufferId);
		vertexBufferId = 0;
	}

	if (indexBufferId)
	{
		glDeleteBuffers(1, &indexBufferId);
		indexBufferId = 0;
	}

	if (shaderProgramId && vertexShaderId)
	{
		glDetachShader(shaderProgramId, vertexShaderId);
	}

	if (shaderProgramId && fragmentShaderId)
	{
		glDetachShader(shaderProgramId, fragmentShaderId);
	}

	if (vertexShaderId)
	{
		glDeleteShader(vertexShaderId);
		vertexShaderId = 0;
	}

	if (fragmentShaderId)
	{
		glDeleteShader(fragmentShaderId);
		fragmentShaderId = 0;
	}

	if (shaderProgramId)
	{
		glDeleteProgram(shaderProgramId);
		shaderProgramId = 0;
	}

	if (fontTexture)
	{
		ImGuiIO& io = ImGui::GetIO();
		glDeleteTextures(1, &fontTexture);
		io.Fonts->SetTexID(0);
		fontTexture = 0;
	}
}

void ImGuiRenderBackend::NewFrame()
{
	if (shaderProgramId == 0)
	{
		CreateDeviceObjects();
	}
}

void ImGuiRenderBackend::RenderDrawData(ImDrawData* draw_data)
{
	// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
	int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
	int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
	if (fb_width <= 0 || fb_height <= 0)
		return;

	// Setup desired GL state
	// Recreate the VAO every time (this is to easily allow multiple GL contexts to be rendered to. VAO are not shared among GL contexts)
	// The renderer would actually work without any VAO bound, but then our VertexAttrib calls would overwrite the default one currently bound.
	GLuint vertex_array_object = 0;
	glGenVertexArrays(1, &vertex_array_object);
	SetupRenderState(draw_data, fb_width, fb_height, vertex_array_object);

	// Will project scissor/clipping rectangles into framebuffer space
	ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
	ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

	// Render command lists
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];

		// Upload vertex/index buffers
		glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * (int)sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * (int)sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback != NULL)
			{
				// User callback, registered via ImDrawList::AddCallback()
				// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
				if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
					SetupRenderState(draw_data, fb_width, fb_height, vertex_array_object);
				else
					pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				// Project scissor/clipping rectangles into framebuffer space
				ImVec4 clip_rect;
				clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
				clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
				clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
				clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

				if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
				{
					// Apply scissor/clipping rectangle
					glScissor((int)clip_rect.x, (int)(fb_height - clip_rect.w), (int)(clip_rect.z - clip_rect.x), (int)(clip_rect.w - clip_rect.y));

					// Bind texture, Draw
					glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
					glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void*)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx)), (GLint)pcmd->VtxOffset);
				}
			}
		}
	}

	// Destroy the temporary VAO
	glDeleteVertexArrays(1, &vertex_array_object);
}

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

static bool CheckProgram(GLuint handle, const char* desc)
{
	GLint status = 0, log_length = 0;
	glGetProgramiv(handle, GL_LINK_STATUS, &status);
	glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_length);
	if ((GLboolean)status == GL_FALSE)
		fprintf(stderr, "ERROR: ImGui_ImplOpenGL3_CreateDeviceObjects: failed to link %s!\n", desc);
	if (log_length > 1)
	{
		ImVector<char> buf;
		buf.resize((int)(log_length + 1));
		glGetProgramInfoLog(handle, log_length, NULL, (GLchar*)buf.begin());
		fprintf(stderr, "%s\n", buf.begin());
	}
	return (GLboolean)status == GL_TRUE;
}

void ImGuiRenderBackend::CreateDeviceObjects()
{
	const GLchar* vertex_shader =
		"#version 130\n"
		"uniform mat4 ProjMtx;\n"
		"in vec2 Position;\n"
		"in vec2 UV;\n"
		"in vec4 Color;\n"
		"out vec2 Frag_UV;\n"
		"out vec4 Frag_Color;\n"
		"void main()\n"
		"{\n"
		"    Frag_UV = UV;\n"
		"    Frag_Color = Color;\n"
		"    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
		"}\n";

	const GLchar* fragment_shader =
		"#version 130\n"
		"uniform sampler2D Texture;\n"
		"in vec2 Frag_UV;\n"
		"in vec4 Frag_Color;\n"
		"out vec4 Out_Color;\n"
		"void main()\n"
		"{\n"
		"    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
		"}\n";

	// Create shaders
	vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderId, 1, &vertex_shader, NULL);
	glCompileShader(vertexShaderId);
	CheckShader(vertexShaderId, "vertex shader");

	fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderId, 1, &fragment_shader, NULL);
	glCompileShader(fragmentShaderId);
	CheckShader(fragmentShaderId, "fragment shader");

	shaderProgramId = glCreateProgram();
	glAttachShader(shaderProgramId, vertexShaderId);
	glAttachShader(shaderProgramId, fragmentShaderId);
	glLinkProgram(shaderProgramId);
	CheckProgram(shaderProgramId, "shader program");

	uniformLocationTex = glGetUniformLocation(shaderProgramId, "Texture");
	uniformLocationProj = glGetUniformLocation(shaderProgramId, "ProjMtx");
	attribLocationPos = (GLuint)glGetAttribLocation(shaderProgramId, "Position");
	attribLocationUV = (GLuint)glGetAttribLocation(shaderProgramId, "UV");
	attribLocationCol = (GLuint)glGetAttribLocation(shaderProgramId, "Color");

	// Create buffers
	glGenBuffers(1, &vertexBufferId);
	glGenBuffers(1, &indexBufferId);

	// Build texture atlas
	ImGuiIO& io = ImGui::GetIO();
	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bit (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

	// Upload texture to graphics system
	glGenTextures(1, &fontTexture);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	// Store our identifier
	io.Fonts->SetTexID((ImTextureID)(intptr_t)fontTexture);
}

void ImGuiRenderBackend::SetupRenderState(ImDrawData* draw_data, int fb_width, int fb_height, unsigned int vertexArray)
{
	// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_SCISSOR_TEST);

	// Setup viewport, orthographic projection matrix
	// Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
	glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
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
	glUseProgram(shaderProgramId);
	glUniform1i(uniformLocationTex, 0);
	glUniformMatrix4fv(uniformLocationProj, 1, GL_FALSE, &ortho_projection[0][0]);

	glBindSampler(0, 0); // We use combined texture/sampler state. Applications using GL 3.3 may set that otherwise.

	glBindVertexArray(vertexArray);

	// Bind vertex/index buffers and setup attributes for ImDrawVert
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
	glEnableVertexAttribArray(attribLocationPos);
	glEnableVertexAttribArray(attribLocationUV);
	glEnableVertexAttribArray(attribLocationCol);
	glVertexAttribPointer(attribLocationPos, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
	glVertexAttribPointer(attribLocationUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
	glVertexAttribPointer(attribLocationCol, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, col));
}
