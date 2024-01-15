#pragma once

#include <string>
#include <vector>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"

class ImGuiPopUp
{
public:
	int IntData = 0;
	bool BoolData = false;

	bool& IsOpen();
	bool& IsCompleted();
	void Reset();

	void Begin();
	bool BeginPopupModal();
	void End();

	void UpdateScale(float Scale);

	std::vector<std::string>& GetData();

	ImGuiPopUp(std::string Title, int SizeX, int SizeY, int DataSize);
private:
	bool m_Open = false;
	bool m_Completed = false;
	std::string m_Title;
	ImVec2 m_Size;

	std::vector<std::string> m_Data;
};