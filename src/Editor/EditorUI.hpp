#pragma once

struct GLFWwindow;

namespace EditorUI
{
	void Initialize(GLFWwindow* window);
	void StartFrame();
	void Render();
	void Deinitialize();
}
