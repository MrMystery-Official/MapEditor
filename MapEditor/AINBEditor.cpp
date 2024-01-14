#include "AINBEditor.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"

#include "tinyfiledialogs.h"

ImGuiPopUp AddAINBNodePopUp("Add node", 500, 210, 1);

void AINBEditor::LoadAINB(std::string Path) {
    if (!this->m_File.Loaded)
    {
        AddAINBNodePopUp.UpdateScale(Config::UIScale);
    }

    this->m_File = AINBFile(Path);

    this->m_GuiNodes.clear();
    for (AINBFile::Node& node : this->m_File.Nodes) {
        this->m_GuiNodes.emplace_back(&node);
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
    ImGui::SameLine();
    if (ImGui::Button("Save"))
    {
        this->m_File.Write(Config::GetWorkingDirFile("Save/Logic/" + this->m_File.Header.FileName + ".ainb"));
    }
    if (ImGui::Button("Add node"))
    {
        AddAINBNodePopUp.IsOpen() = true;
    }
    if (AddAINBNodePopUp.IsCompleted())
    {

        AINBFile::Node Node;
        Node.NodeIndex = this->m_File.Nodes.size();
        Node.Type = (uint16_t)AINBFile::NodeTypes::UserDefined;
        Node.AttachmentCount = 0;
        Node.Name = AddAINBNodePopUp.GetData()[0];

        Node.InputParameters.resize(6);
        Node.OutputParameters.resize(6);
        Node.ImmediateParameters.resize(6);

        AINBNodeDefinitions::NodeDefinition* Definition = AINBNodeDefinitions::GetNodeDefinition(Node.Name);

        if (Definition != nullptr)
        {
            Node.NameHash = Definition->NameHash;
            Node.Type = Definition->Type;
            for (int Type = 0; Type < AINBFile::ValueTypeCount; Type++)
            {
                for (AINBNodeDefinitions::NodeDefinitionOutputParameter Param : Definition->OutputParameters[Type])
                {
                    AINBFile::OutputEntry Entry;
                    Entry.Name = Param.Name;
                    Entry.Class = Param.Class;
                    Entry.SetPointerFlagsBitZero = Param.SetPointerFlagsBitZero;
                    Node.OutputParameters[Type].push_back(Entry);
                }
                for (AINBNodeDefinitions::NodeDefinitionImmediateParameter Param : Definition->ImmediateParameters[Type])
                {
                    AINBFile::ImmediateParameter Entry;
                    Entry.Name = Param.Name;
                    Entry.Class = Param.Class;
                    Entry.ValueType = (int)Param.ValueType;
                    switch (Param.ValueType)
                    {
                    case AINBFile::ValueType::Bool:
                        Entry.Value = false;
                        break;
                    case AINBFile::ValueType::Int:
                        Entry.Value = (uint32_t)0;
                        break;
                    case AINBFile::ValueType::Float:
                        Entry.Value = 0.0f;
                        break;
                    case AINBFile::ValueType::String:
                        Entry.Value = "None";
                        break;
                    case AINBFile::ValueType::Vec3f:
                        Entry.Value = Vector3F(0.0f, 0.0f, 0.0f);
                        break;
                    }
                    Node.ImmediateParameters[Type].push_back(Entry);
                }
                for (AINBNodeDefinitions::NodeDefinitionInputParameter Param : Definition->InputParameters[Type])
                {
                    AINBFile::InputEntry Entry;
                    Entry.Name = Param.Name;
                    Entry.NodeIndex = -1;
                    Entry.ParameterIndex = -1;
                    Entry.Class = Param.Class;
                    Entry.ValueType = (int)Param.ValueType;
                    Entry.Value = Param.Value;
                    Node.InputParameters[Type].push_back(Entry);
                }
            }

            this->m_File.Nodes.push_back(Node);
            this->m_GuiNodes.clear();
            AINBImGuiNode::NextID = 0;
            for (AINBFile::Node& node : this->m_File.Nodes) {
                this->m_GuiNodes.emplace_back(&node);
            }
        }

        AddAINBNodePopUp.Reset();
    }
    if (AddAINBNodePopUp.IsOpen())
    {
        this->m_NodeNames.clear();
        for (AINBNodeDefinitions::NodeDefinition Def : AINBNodeDefinitions::NodeDefinitions)
        {
            if (Def.Name.find(AddAINBNodePopUp.GetData()[0]) != std::string::npos)
            {
                this->m_NodeNames.push_back(Def.Name);
            }
        }

        AddAINBNodePopUp.Begin();
        if (AddAINBNodePopUp.BeginPopupModal())
        {
            ImGui::InputText("Search", &AddAINBNodePopUp.GetData()[0]);

            int selected = 0;

            if (ImGui::ListBox("##ListBox", &selected,
                [](void* vec, int idx, const char** out_text) {
                    std::vector<std::string>* vector = reinterpret_cast<std::vector<std::string>*>(vec);
                    if (idx < 0 || idx >= vector->size())return false;
                    *out_text = vector->at(idx).c_str();
                    return true;
                }, reinterpret_cast<void*>(&this->m_NodeNames), this->m_NodeNames.size()))
            {
                AddAINBNodePopUp.GetData()[0] = this->m_NodeNames[selected];
            }

            if (ImGui::Button("Add"))
            {
                AddAINBNodePopUp.IsOpen() = false;
                AddAINBNodePopUp.IsCompleted() = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Return"))
            {
                AddAINBNodePopUp.Reset();
            }
        }
        AddAINBNodePopUp.End();
    }

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

    ed::NodeId SelectedNodeID;
    if (ed::GetSelectedNodes(&SelectedNodeID, 1) > 0) {
        for (AINBImGuiNode& guiNode : this->m_GuiNodes) {
            if (guiNode.GetNodeID() == SelectedNodeID) {
                this->m_SelectedNodeIdx = guiNode.GetNode().NodeIndex;
                break;
            }
        }
    }
    else
    {
        this->m_SelectedNodeIdx = -1;
    }

    if (!ed::GetSelectedLinks(&this->m_SelectedLink, 1))
    {
        this->m_SelectedLink = 0;
    }

    ed::Resume();

    if (wantAutoLayout) {
        AutoLayout();
    }

    if (ed::BeginCreate())
    {
        ed::PinId InputPinID, OutputPinID;
        if (ed::QueryNewLink(&InputPinID, &OutputPinID))
        {
            if (InputPinID && OutputPinID) // both are valid, let's accept link
            {
                // ed::AcceptNewItem() return true when user release mouse button.
                if (ed::AcceptNewItem())
                {

                    int NodeIndex = 0;
                    int ParameterIndex = 0;

                    for (AINBImGuiNode& GuiNode : this->m_GuiNodes)
                    {
                        int Index = 0;
                        for (auto const& [Key, Val] : GuiNode.IdxToID[1])
                        {
                            if (Val.Get() == InputPinID.Get())
                            {
                                std::cout << "Output Node: " << GuiNode.GetNode().Name << std::endl;
                                NodeIndex = GuiNode.GetNode().NodeIndex;
                                ParameterIndex = Index;
                                goto FoundOutput;
                            }
                            Index++;
                        }
                    }

                FoundOutput:

                    for (AINBImGuiNode& GuiNode : this->m_GuiNodes)
                    {
                        for (auto const& [Key, Val] : GuiNode.IdxToID[0])
                        {
                            if (Val.Get() == OutputPinID.Get())
                            {
                                std::cout << "Input Node: " << GuiNode.GetNode().Name << std::endl;
                                int Index = 0;
                                for (int Type = 0; Type < AINBFile::ValueTypeCount; Type++)
                                {
                                    for (AINBFile::InputEntry& Param : GuiNode.GetNode().InputParameters[Type])
                                    {
                                        std::cout << Index << ", " << Key << std::endl;
                                        if (Index == Key)
                                        {
                                            std::cout << "SET\n";
                                            Param.NodeIndex = NodeIndex;
                                            Param.ParameterIndex = ParameterIndex;
                                            GuiNode.UpdateLink(Param, Key);
                                            goto FoundInput;
                                        }
                                        Index++;
                                    }
                                }
                            }
                        }
                    }
                FoundInput:;
                }
            }
        }
    }
    ed::EndCreate(); // Wraps up object creation action handling.

    ed::End();
    ed::SetCurrentEditor(nullptr);
}

void AINBEditor::DrawProperties()
{
    if (this->m_SelectedLink.Get() != 0)
    {
        ImGui::Text(("Link: " + std::to_string(this->m_SelectedLink.Get())).c_str());
        ImGui::SameLine();
        if (ImGui::Button("Delete"))
        {
            for (AINBImGuiNode& GuiNode : this->m_GuiNodes)
            {
                for (AINBImGuiNode::ParamLink& Link : GuiNode.ParamLinks)
                {
                    if (Link.LinkID.Get() == this->m_SelectedLink.Get())
                    {
                        Link.InputParam->NodeIndex = -1;
                        Link.InputParam->ParameterIndex = -1;
                        this->m_SelectedLink = 0;
                        GuiNode.UpdateLink(*Link.InputParam, Link.InputParamIndex);
                        goto FoundOutput;
                    }
                }
            }
        FoundOutput:
            return;
        }
    }

    if (this->m_SelectedNodeIdx != -1)
    {
        ImGui::Text(("Node: " + std::to_string(this->m_SelectedNodeIdx)).c_str());
        ImGui::SameLine();
        if (ImGui::Button("Delete"))
        {
            this->m_File.Nodes.erase(this->m_File.Nodes.begin() + this->m_SelectedNodeIdx);
            this->m_GuiNodes.erase(this->m_GuiNodes.begin() + this->m_SelectedNodeIdx);
            this->m_SelectedNodeIdx = -1;

            for (AINBImGuiNode& Node : this->m_GuiNodes)
            {
                for (AINBImGuiNode::ParamLink& Link : Node.ParamLinks)
                {
                    if (Link.InputNodeIdx == this->m_SelectedNodeIdx)
                    {
                        Link.InputParam->NodeIndex = -1;
                        Link.InputParam->ParameterIndex = -1;
                        Node.UpdateLink(*Link.InputParam, Link.InputParamIndex);
                    }
                }
            }
        }
    }
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