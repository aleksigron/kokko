#pragma once

struct ImDrawData;

class ImGuiRenderBackend
{
private:
	unsigned int fontTexture;
	unsigned int shaderProgramId;
	unsigned int vertexShaderId;
	unsigned int fragmentShaderId;
	int uniformLocationTex;
	int uniformLocationProj;
	unsigned int attribLocationPos;
	unsigned int attribLocationUV;
	unsigned int attribLocationCol;
	unsigned int vertexBufferId;
	unsigned int indexBufferId;

	void CreateDeviceObjects();
	void SetupRenderState(ImDrawData* draw_data, int fb_width, int fb_height, unsigned int vertexArray);

public:
	ImGuiRenderBackend();
	ImGuiRenderBackend(const ImGuiRenderBackend&) = delete;
	ImGuiRenderBackend(ImGuiRenderBackend&&) = delete;
	~ImGuiRenderBackend();

	ImGuiRenderBackend& operator=(const ImGuiRenderBackend&) = delete;
	ImGuiRenderBackend& operator=(ImGuiRenderBackend&&) = delete;

	bool Initialize();
	void Deinitialize();
	void NewFrame();
	void RenderDrawData(ImDrawData* draw_data);

};
