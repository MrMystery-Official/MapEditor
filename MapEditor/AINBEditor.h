#pragma once

#include "AINB.h"
#include "imgui_node_editor/imnodes.h"
#include "Config.h"
#include "ImGuiPopUp.h"
#include "AINBNodeDefinitions.h"

class AINBEditor
{
public:
	
	void DrawNodeEditor();

	AINBEditor(std::string Path);
	AINBEditor() {}
private:
	AINBFile m_File;
	int CurrentID = 0;
	std::vector<std::string> m_NodeNames;

	void SetNodePos(AINBFile::Node* Node, int WidthOffset, int HeightOffset);
	void DrawPreconditionNode(AINBFile::InputEntry& Parameter);
	void DrawNode(AINBFile::Node& Node);
};