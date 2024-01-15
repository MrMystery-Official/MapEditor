#include "ImGuiPopUp.h"

bool& ImGuiPopUp::IsOpen()
{
	return this->m_Open;
}

bool& ImGuiPopUp::IsCompleted()
{
	return this->m_Completed;
}

void ImGuiPopUp::Reset()
{
	this->m_Completed = false;
	this->m_Open = false;
	this->m_Data.clear();
	this->IntData = 0;
	this->BoolData = false;
}

void ImGuiPopUp::UpdateScale(float Scale)
{
	this->m_Size.x *= Scale;
	this->m_Size.y *= Scale;
}

void ImGuiPopUp::Begin()
{
	ImGui::SetNextWindowSize(this->m_Size);
	ImGui::OpenPopup(this->m_Title.c_str());
}

bool ImGuiPopUp::BeginPopupModal()
{
	//return ImGui::BeginPopupModal(this->m_Title.c_str(), NULL, ImGuiWindowFlags_NoResize);
	return ImGui::BeginPopupModal(this->m_Title.c_str());
}

void ImGuiPopUp::End()
{
	ImGui::SameLine();
	ImGui::Text(std::string("Size: " + std::to_string(ImGui::GetWindowSize().x) + "x" + std::to_string(ImGui::GetWindowSize().y)).c_str());
	ImGui::EndPopup();
}

std::vector<std::string>& ImGuiPopUp::GetData()
{
	return this->m_Data;
}

ImGuiPopUp::ImGuiPopUp(std::string Title, int SizeX, int SizeY, int DataSize)
{
	this->m_Title = Title;
	this->m_Size = ImVec2(SizeX, SizeY);
	this->m_Data.resize(DataSize);
}