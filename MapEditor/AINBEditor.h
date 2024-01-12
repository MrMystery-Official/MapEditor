#pragma once

#include "AINB.h"
#include "imgui_node_editor/imgui_node_editor.h"
#include "Config.h"
#include "ImGuiPopUp.h"
#include "AINBNodeDefinitions.h"
#include <functional>
#include "AINBNode.h"

namespace ed = ax::NodeEditor;

class AINBEditor
{
public:
	AINBFile m_File;

	void LoadAINB(std::string Path);
	void DrawNodeEditor();
	void DrawProperties();
	void Initialize();
	void Destroy();
private:
	ed::EditorContext* m_Context = nullptr;
	std::vector<AINBImGuiNode> m_GuiNodes;
	ed::NodeId m_RightClickedNode = 0;
	size_t m_SelectedNodeIdx = -1;
	std::string m_SelectedCommand = "";
	ed::LinkId m_SelectedLink;
	std::unordered_map<int, AINBImGuiNode::AuxInfo> m_NewAuxInfos;

	void AutoLayout();
};