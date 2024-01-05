#pragma once

#include "AINB.h"
#include "imgui_node_editor/imnodes.h"

class AINBEditor
{
public:
	
	void DrawNodeEditor();

	AINBEditor(std::string Path);
	AINBEditor() {}
private:
	AINBFile m_File;

	void SetNodePos(AINBFile::Node* Node, int WidthOffset, int HeightOffset);
	void DrawPreconditionNode(AINBFile::InputEntry& Parameter);
	void DrawNode(AINBFile::Node& Node);
};