#pragma once

#include "AINB.h"
#include "imgui_node_editor/imnodes.h"
#include "Config.h"
#include "ImGuiPopUp.h"
#include "AINBNodeDefinitions.h"
#include <functional>

class AINBEditor
{
public:
	
	void DrawNodeEditor();
	void Initialize();
private:
	AINBFile m_File;
	int CurrentID = 0;
	std::vector<std::string> m_NodeNames;
	bool OriginalFileExists = false;

	bool SetNodePos(uint16_t NodeIndex, int WidthOffset, int HeightOffset, std::vector<uint16_t> Indexes);
	void DrawPreconditionNode(AINBFile::InputEntry& Parameter);
	void DrawNode(AINBFile::Node& Node);
};