#include "AINBEditor.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"

#include "tinyfiledialogs.h"

void AINBEditor::LoadAINB(std::string Path) {
    this->m_File = AINBFile(Path);
    this->m_GuiNodes.clear();
    for (AINBFile::Node& node : this->m_File.Nodes) {
        this->m_GuiNodes.emplace_back(node);
    }

    if (this->m_Context != nullptr) {
        ed::DestroyEditor(this->m_Context);
    }
    ed::Config Config;
    Config.SettingsFile = nullptr;
    this->m_Context = ed::CreateEditor(&Config);
    this->m_SelectedNodeIdx = -1;
    this->m_SelectedCommand = "";
}

void AINBEditor::AutoLayout() {
    // Use a very simple layouting strategy for now
    std::unordered_map<int, std::pair<int, int>> IdxToPos;
    std::unordered_map<int, std::unordered_map<int, int>> PosToIdx;

    std::function<void(int, std::pair<int, int>)> PlaceNode;

    PlaceNode = [&](uint16_t NodeIdx, std::pair<int, int> Pos) {
        if (IdxToPos.contains(NodeIdx)) {
            return;
        }
        while (PosToIdx[Pos.first].contains(Pos.second)) {
            Pos.second++;
        }
        IdxToPos[NodeIdx] = Pos;
        PosToIdx[Pos.first][Pos.second] = NodeIdx;

        for (AINBFile::Node& Node : this->m_File.Nodes)
        {
            for (int j = 0; j < AINBFile::ValueTypeCount; j++)
            {
                for (AINBFile::InputEntry Param : Node.InputParameters[j]) {
                    if (Param.NodeIndex == NodeIdx) PlaceNode(Param.NodeIndex, { Pos.first - 1, Pos.second });
                }
            }
        }
        for (int j = 0; j < AINBFile::ValueTypeCount; j++)
        {
            for (AINBFile::InputEntry Param : this->m_File.Nodes[NodeIdx].InputParameters[j]) {
                if (Param.NodeIndex >= 0) PlaceNode(Param.NodeIndex, { Pos.first - 1, Pos.second });
            }
        }
        };

    // First, try to place command root nodes
    for (size_t i = 0; i < this->m_File.Commands.size(); i++) {
        PlaceNode(this->m_File.Nodes[this->m_File.Commands[i].LeftNodeIndex].NodeIndex, {i, 0});
    }

    // Place remaining orphan nodes
    for (const AINBFile::Node& Node : this->m_File.Nodes) {
        if (!IdxToPos.contains(Node.NodeIndex)) {
            PlaceNode(Node.NodeIndex, { 0, 0 });
        }
    }

    this->m_NewAuxInfos.clear();
    for (auto& [nodeIdx, coord] : IdxToPos) {
        AINBImGuiNode::AuxInfo auxInfo;
        auxInfo.NodeIdx = nodeIdx;
        auxInfo.Pos = ImVec2(coord.first * 600, coord.second * 400);

        int extraPinIdx = 0;
        for (int Type = 0; Type < AINBFile::ValueTypeCount; Type++)
        {
            for (AINBFile::InputEntry& Param : this->m_File.Nodes[nodeIdx].InputParameters[Type])
            {
                if (Param.NodeIndex < 0)
                {
                    auxInfo.ExtraNodePos[Param.Name] = ImVec2(auxInfo.Pos.x - 250, auxInfo.Pos.y + extraPinIdx * 70);
                    extraPinIdx++;
                }
            }
        }
        this->m_NewAuxInfos[nodeIdx] = auxInfo;
    }
}

void AINBEditor::DrawNodeEditor()
{
    bool wantAutoLayout = ImGui::Button("Auto layout");
    ImGui::Separator();

    ed::SetCurrentEditor(this->m_Context);
    ed::Begin("AINB Editor", ImVec2(0.0, 0.0f));

    if (this->m_NewAuxInfos.size() > 0) {
        for (AINBImGuiNode& guiNode : this->m_GuiNodes) {
            if (this->m_NewAuxInfos.contains(guiNode.GetNode().NodeIndex)) {
                guiNode.LoadAuxInfo(this->m_NewAuxInfos[guiNode.GetNode().NodeIndex]);
            }
        }
        this->m_NewAuxInfos.clear();
    }

    for (AINBImGuiNode& guiNode : this->m_GuiNodes) {
        guiNode.Draw();
    }
    for (AINBImGuiNode& guiNode : this->m_GuiNodes) {
        guiNode.DrawLinks(this->m_GuiNodes);
    }

    ed::Suspend();

    if (ed::ShowNodeContextMenu(&this->m_RightClickedNode)) {
        ImGui::OpenPopup("Node Actions");
    }

    if (ImGui::BeginPopup("Node Actions")) {
        if (ImGui::MenuItem("Copy default value")) {
            for (const AINBImGuiNode& guiNode : this->m_GuiNodes) {
                for (const AINBImGuiNode::NonNodeInput& input : guiNode.GetNonNodeInputs()) {
                    if (input.GenNodeID == this->m_RightClickedNode) {
                        ImGui::SetClipboardText(AINBFile::ValueToString(input.InputParam.Value).c_str());
                        goto found;
                    }
                }
            }
        found:;
        }
        ImGui::EndPopup();
    }

    ed::NodeId selectedNodeID;
    if (ed::GetSelectedNodes(&selectedNodeID, 1) > 0) {
        for (AINBImGuiNode& guiNode : this->m_GuiNodes) {
            if (guiNode.GetNodeID() == selectedNodeID) {
                this->m_SelectedNodeIdx = guiNode.GetNode().NodeIndex;
                break;
            }
        }
    }
    else
    {
        this->m_SelectedNodeIdx = -1;
    }

    ed::Resume();

    if (wantAutoLayout) {
        AutoLayout();
    }

    ed::End();
    ed::SetCurrentEditor(nullptr);
}

void AINBEditor::Initialize()
{
    ed::Config Config;
    Config.SettingsFile = nullptr;
    this->m_Context = ed::CreateEditor(&Config);
}

void AINBEditor::Destroy()
{
    ed::DestroyEditor(this->m_Context);
}