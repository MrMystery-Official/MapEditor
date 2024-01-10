#include "AINBEditor.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"

#include "tinyfiledialogs.h"

void AINBEditor::DrawNode(AINBFile::Node& Node)
{
    ImNodes::BeginNode(Node.EditorID);

    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted(Node.Name.c_str());
    ImNodes::EndNodeTitleBar();

    float WidthInput = ImGui::CalcTextSize(Node.Name.c_str()).x;
    float WidthOutput = 0;

    for (int i = 0; i < this->m_File.ValueTypeCount; i++)
    {
        for (int j = 0; j < Node.InputParameters[i].size(); j++)
        {
            AINBFile::InputEntry Parameter = Node.InputParameters[i][j];
            WidthInput = std::max(WidthInput, ImGui::CalcTextSize(Parameter.Name.c_str()).x);
        }
        for (int j = 0; j < Node.ImmediateParameters[i].size(); j++)
        {
            AINBFile::ImmediateParameter& Parameter = Node.ImmediateParameters[i][j];
            float Size = ImGui::CalcTextSize(Parameter.Name.c_str()).x;
            switch (i)
            {
            case (uint32_t)AINBFile::ValueType::Float:
                Size += ImGui::CalcTextSize(std::to_string(*reinterpret_cast<float*>(&Parameter.Value)).c_str()).x;
                break;
            case (uint32_t)AINBFile::ValueType::Int:
                Size += ImGui::CalcTextSize(std::to_string(*reinterpret_cast<int*>(&Parameter.Value)).c_str()).x;
                break;
            case (uint32_t)AINBFile::ValueType::String:
                Size += ImGui::CalcTextSize(reinterpret_cast<std::string*>(&Parameter.Value)->c_str()).x;
                break;
                /*
            case (uint32_t)AINBFile::ValueType::Vec3f:
                ImGui::InputFloat3(("##" + Parameter.Name).c_str(), reinterpret_cast<Vector3F*>(&Parameter.Value)->GetRawData());
                break;
                */
            }
            WidthInput = std::max(WidthInput, Size);
        }
        ImVec2 windowSize = ImNodes::GetNodeDimensions(Node.EditorID);
        for (int j = 0; j < Node.OutputParameters[i].size(); j++)
        {
            AINBFile::OutputEntry Parameter = Node.OutputParameters[i][j];
            WidthOutput = std::max(WidthOutput, ImGui::CalcTextSize(Parameter.Name.c_str()).x);
        }
    }

    float Width = WidthInput + WidthOutput + 14;

    for (int i = 0; i < this->m_File.ValueTypeCount; i++)
    {
        for (int j = 0; j < Node.InputParameters[i].size(); j++)
        {
            AINBFile::InputEntry& Parameter = Node.InputParameters[i][j];

            ImNodes::BeginInputAttribute(Parameter.EditorID);
            ImGui::Text(Parameter.Name.c_str());
            ImNodes::EndInputAttribute();
        }
        for (int j = 0; j < Node.ImmediateParameters[i].size(); j++)
        {
            AINBFile::ImmediateParameter& Parameter = Node.ImmediateParameters[i][j];
            ImGui::Text(Parameter.Name.c_str());
            
            ImGui::SameLine();
            
            switch (i)
            {
            case (uint32_t)AINBFile::ValueType::Bool:
                ImGui::Checkbox(("##" + Parameter.Name).c_str(), reinterpret_cast<bool*>(&Parameter.Value));
                break;
            case (uint32_t)AINBFile::ValueType::Float:
                ImGui::InputFloat(("##" + Parameter.Name).c_str(), reinterpret_cast<float*>(&Parameter.Value));
                break;
            case (uint32_t)AINBFile::ValueType::Int:
                ImGui::InputInt(("##" + Parameter.Name).c_str(), reinterpret_cast<int*>(&Parameter.Value));
                break;
            case (uint32_t)AINBFile::ValueType::String:
                ImGui::PushItemWidth(Width - ImGui::CalcTextSize(Parameter.Name.c_str()).x - 20.0f);
                ImGui::InputText(("##" + Parameter.Name).c_str(), reinterpret_cast<std::string*>(&Parameter.Value));
                ImGui::PopItemWidth();
                break;
                /*
            case (uint32_t)AINBFile::ValueType::Vec3f:
                ImGui::InputFloat3(("##" + Parameter.Name).c_str(), reinterpret_cast<Vector3F*>(&Parameter.Value)->GetRawData());
                break;
                */
            }
            
        }
        for (int j = 0; j < Node.OutputParameters[i].size(); j++)
        {
            AINBFile::OutputEntry& Parameter = Node.OutputParameters[i][j];
            ImNodes::BeginOutputAttribute(Parameter.EditorID);
            ImGui::Indent(Width - ImGui::CalcTextSize(Parameter.Name.c_str()).x);
            ImGui::Text(Parameter.Name.c_str());
            ImNodes::EndOutputAttribute();
        }
    }

    ImNodes::EndNode();
}

void AINBEditor::DrawPreconditionNode(AINBFile::InputEntry& Parameter)
{
    ImNodes::PushColorStyle(
        ImNodesCol_TitleBar, IM_COL32(39, 174, 96, 255));
    ImNodes::PushColorStyle(
        ImNodesCol_TitleBarSelected, IM_COL32(66, 224, 133, 255));
    ImNodes::PushColorStyle(
        ImNodesCol_TitleBarHovered, IM_COL32(66, 224, 133, 255));

    ImNodes::PushColorStyle(
        ImNodesCol_Pin, IM_COL32(39, 174, 96, 255));
    ImNodes::PushColorStyle(
        ImNodesCol_PinHovered, IM_COL32(66, 224, 133, 255));

    ImNodes::BeginNode(Parameter.EditorID + 1);

    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted(("Value: " + Parameter.Name + " (" + AINBFile::StandardTypeToString(Parameter.ValueType) + ")").c_str());
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginOutputAttribute(Parameter.EditorID + 2);
    //ImGui::Text(AINBFile::AINBValueToString(Node.Value).c_str());

    
    switch (Parameter.ValueType)
    {
    case (int)AINBFile::ValueType::Bool:
        ImGui::Indent(ImGui::CalcTextSize(("Value: " + Parameter.Name + " (" + AINBFile::StandardTypeToString(Parameter.ValueType) + ")").c_str()).x - 16.0f);
        ImGui::Checkbox(("##" + Parameter.Name).c_str(), reinterpret_cast<bool*>(&Parameter.Value));
        break;
    case (int)AINBFile::ValueType::Float:
        ImGui::PushItemWidth(ImGui::CalcTextSize(("Value: " + Parameter.Name + " (" + AINBFile::StandardTypeToString(Parameter.ValueType) + ")").c_str()).x);
        ImGui::InputScalar(("##" + Parameter.Name).c_str(), ImGuiDataType_::ImGuiDataType_Float, &Parameter.Value);
        break;
    case (int)AINBFile::ValueType::Int:
        ImGui::PushItemWidth(ImGui::CalcTextSize(("Value: " + Parameter.Name + " (" + AINBFile::StandardTypeToString(Parameter.ValueType) + ")").c_str()).x);
        ImGui::InputScalar(("##" + Parameter.Name).c_str(), ImGuiDataType_::ImGuiDataType_U32, &Parameter.Value);
        ImGui::PopItemWidth();
        break;
    case (int)AINBFile::ValueType::String:
        ImGui::PushItemWidth(ImGui::CalcTextSize(("Value: " + Parameter.Name + " (" + AINBFile::StandardTypeToString(Parameter.ValueType) + ")").c_str()).x);
        ImGui::InputText(("##" + Parameter.Name).c_str(), reinterpret_cast<std::string*>(&Parameter.Value));
        ImGui::PopItemWidth();
        break;
        /*
    case AINBFile::ValueType::Vec3f:
        ImGui::InputFloat3(("##" + Node.Name).c_str(), reinterpret_cast<Vector3F*>(&Node.DestNodeParameter.DefaultValue)->GetRawData());
        break;
        */
    }
    

    ImNodes::EndOutputAttribute();

    ImNodes::EndNode();

    ImNodes::PopColorStyle();
    ImNodes::PopColorStyle();
    ImNodes::PopColorStyle();
    ImNodes::PopColorStyle();
    ImNodes::PopColorStyle();
}

bool AINBEditor::SetNodePos(uint16_t NodeIndex, int WidthOffset, int HeightOffset, std::vector<uint16_t> Indexes)
{

    return true;
}

ImGuiPopUp AddAINBNodePopUp("Add node", 500, 210, 1);
ImGuiPopUp ConfirmAINBRevert("Confirm revert", 200, 200, 0);
ImGuiPopUp CreateNewAINBFile("New", 400, 100, 1);

void AINBEditor::DrawNodeEditor()
{
    bool WantAutoLayout = false;
    bool WantDeleteSelected = false;
    if (ImGui::Button("Open vanilla"))
    {
        const char* Path = tinyfd_openFileDialog("Open file", (Config::RomFSPath + "/Logic").c_str(), 0, nullptr, nullptr, 0);
        if (Path != nullptr) {
            this->m_File = AINBFile(Config::GetRomFSFile("Logic/" + std::string(strrchr(Path, '\\') + 1)));

            this->CurrentID = 0;
            this->m_NodeNames.clear();
            this->OriginalFileExists = Util::FileExists(Config::GetRomFSFile("Logic/" + this->m_File.Header.FileName + ".ainb", false));

            for (AINBFile::Node& Node : this->m_File.Nodes)
            {
                Node.EditorID = CurrentID++;
                for (int i = 0; i < AINBFile::ValueTypeCount; i++)
                {
                    for (AINBFile::InputEntry& Param : Node.InputParameters[i])
                    {
                        Param.EditorID = CurrentID++;
                        CurrentID += 3;
                    }
                    for (AINBFile::OutputEntry& Param : Node.OutputParameters[i])
                    {
                        Param.EditorID = CurrentID++;
                    }
                }
            }
            WantAutoLayout = true;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Open custom"))
    {
        const char* Path = tinyfd_openFileDialog("Open file", Config::GetWorkingDirFile("Save/Logic").c_str(), 0, nullptr, nullptr, 0);
        if (Path != nullptr) {
            this->m_File = AINBFile(Path);

            this->CurrentID = 0;
            this->m_NodeNames.clear();

            for (AINBFile::Node& Node : this->m_File.Nodes)
            {
                Node.EditorID = CurrentID++;
                for (int i = 0; i < AINBFile::ValueTypeCount; i++)
                {
                    for (AINBFile::InputEntry& Param : Node.InputParameters[i])
                    {
                        Param.EditorID = CurrentID++;
                        CurrentID += 3;
                    }
                    for (AINBFile::OutputEntry& Param : Node.OutputParameters[i])
                    {
                        Param.EditorID = CurrentID++;
                    }
                }
            }
            WantAutoLayout = true;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("New"))
    {
        CreateNewAINBFile.IsOpen() = true;
    }
    if (CreateNewAINBFile.IsCompleted())
    {
        this->m_File = AINBFile();
        this->m_File.Header.FileName = CreateNewAINBFile.GetData()[0] + ".logic.root";
        this->m_File.Header.FileCategory = CreateNewAINBFile.IntData == 0 ? "Logic" : (CreateNewAINBFile.IntData == 1 ? "AI" : "Sequence");

        this->m_File.GlobalParameters.resize(6);

        this->m_File.Loaded = true;

        CreateNewAINBFile.Reset();
    }
    if (CreateNewAINBFile.IsOpen())
    {
        CreateNewAINBFile.Begin();
        if (CreateNewAINBFile.BeginPopupModal())
        {
            ImGui::InputText("File name", &CreateNewAINBFile.GetData()[0]);
            const char* CategoryDropdownItems[] = { "Logic", "AI", "Sequence" };
            ImGui::Combo("Category", reinterpret_cast<int*>(&CreateNewAINBFile.IntData), CategoryDropdownItems, IM_ARRAYSIZE(CategoryDropdownItems));
            if (ImGui::Button("Create AINB"))
            {
                CreateNewAINBFile.IsCompleted() = true;
                CreateNewAINBFile.IsOpen() = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Return"))
            {
                CreateNewAINBFile.Reset();
            }

        }
        CreateNewAINBFile.End();
    }

    if (this->m_File.Loaded)
    {
        ImGui::SameLine();

        if (ImGui::Button("Save"))
        {
            this->m_File.Write(Config::GetWorkingDirFile("Save/Logic/" + this->m_File.Header.FileName + ".ainb"));
        }
        if (this->OriginalFileExists)
        {
            ImGui::SameLine();
            if (ImGui::Button("Revert to original"))
            {
                ConfirmAINBRevert.IsOpen() = true;
            }
        }
        if(ConfirmAINBRevert.IsCompleted())
        {
            this->m_File = AINBFile(Config::GetRomFSFile("Logic/" + this->m_File.Header.FileName + ".ainb", false));

            this->CurrentID = 0;
            this->m_NodeNames.clear();

            for (AINBFile::Node& Node : this->m_File.Nodes)
            {
                Node.EditorID = CurrentID++;
                for (int i = 0; i < AINBFile::ValueTypeCount; i++)
                {
                    for (AINBFile::InputEntry& Param : Node.InputParameters[i])
                    {
                        Param.EditorID = CurrentID++;
                        CurrentID += 3;
                    }
                    for (AINBFile::OutputEntry& Param : Node.OutputParameters[i])
                    {
                        Param.EditorID = CurrentID++;
                    }
                }
            }

            WantAutoLayout = true;
            ConfirmAINBRevert.Reset();
        }
        if (ConfirmAINBRevert.IsOpen())
        {
            ConfirmAINBRevert.Begin();
            if (ConfirmAINBRevert.BeginPopupModal())
            {
                ImGui::Text("Do you really want to undo all changes and import the original file?");
                ImGui::NewLine();
                if (ImGui::Button("Yes"))
                {
                    ConfirmAINBRevert.IsCompleted() = true;
                    ConfirmAINBRevert.IsOpen() = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("No"))
                {
                    ConfirmAINBRevert.Reset();
                }
            }
            ConfirmAINBRevert.End();
        }

        if(!WantAutoLayout) WantAutoLayout = ImGui::Button("Auto layout");
        ImGui::SameLine();
        WantDeleteSelected = ImGui::Button("Delete selected");
        ImGui::SameLine();
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
            Node.EditorID = CurrentID++;

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
                        Entry.EditorID = CurrentID++;
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
                        Entry.EditorID = CurrentID++;
                        CurrentID += 3;
                        Node.InputParameters[Type].push_back(Entry);
                    }
                }

                this->m_File.Nodes.push_back(Node);
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
    }

    ImNodes::BeginNodeEditor();

    for (AINBFile::Node& Node : this->m_File.Nodes)
    {
        DrawNode(Node);
    }

    for (AINBFile::Node& Node : this->m_File.Nodes)
    {
        Node.PreconditionCount = 0;
        for (int i = 0; i < AINBFile::ValueTypeCount; i++)
        {
            for (AINBFile::InputEntry& Param : Node.InputParameters[i])
            {
                if (Param.NodeIndex == -1)
                {
                    DrawPreconditionNode(Param);
                    Node.PreconditionCount++;
                }
            }
        }
    }

    if (WantAutoLayout)
    {
        std::cout << "LAYOUT\n";

        std::unordered_map<int, std::pair<int, int>> IdxToPos;
        std::unordered_map<int, std::unordered_map<int, int>> PosToIdx;

        std::function<void(uint16_t, std::pair<int, int>)> PlaceNode;

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
                        if(Param.NodeIndex == NodeIdx) PlaceNode(Param.NodeIndex, { Pos.first - 1, Pos.second });
                    }
                }
            }
            for (int j = 0; j < AINBFile::ValueTypeCount; j++)
            {
                for (AINBFile::InputEntry Param : this->m_File.Nodes[NodeIdx].InputParameters[j]) {
                    if(Param.NodeIndex >= 0) PlaceNode(Param.NodeIndex, { Pos.first - 1, Pos.second });
                }
            }
        };

        for (int i = 0; i < this->m_File.Commands.size(); i++)
        {
            PlaceNode(this->m_File.Nodes[this->m_File.Commands[i].LeftNodeIndex].NodeIndex, { i, 0 });
        }

        for (AINBFile::Node& Node : this->m_File.Nodes) {
            PlaceNode(Node.NodeIndex, { 0, 0 });
        }

        for (auto& [NodeIdx, Coord] : IdxToPos) {
            ImNodes::SetNodeGridSpacePos(this->m_File.Nodes[NodeIdx].EditorID, ImVec2(Coord.first * 600, Coord.second * 400));

            for (int i = 0; i < AINBFile::ValueTypeCount; i++)
            {
                for (AINBFile::InputEntry Param : this->m_File.Nodes[NodeIdx].InputParameters[i])
                {
                    if (Param.NodeIndex == -1)
                    {
                        ImNodes::SetNodeGridSpacePos(Param.EditorID + 1, ImVec2(Coord.first * 600, Coord.second * 400));
                    }
                }
            }

            /*
            int extraPinIdx = 0;
            for (AINB::Param& param : ainb->nodes[nodeIdx].GetParams()) {
                if (param.paramType == AINB::ParamType::Input) {
                    const AINB::InputParam& inputParam = static_cast<const AINB::InputParam&>(param);
                    if (inputParam.inputNodeIdxs.size() == 0) {
                        auxInfo.extraNodePos[param.name] = ImVec2(auxInfo.pos.x - 250, auxInfo.pos.y + extraPinIdx * 70);
                        extraPinIdx++;
                    }
                }
            }
            newAuxInfos[nodeIdx] = auxInfo;
            */
        }
    }

    for (AINBFile::Node& Node : this->m_File.Nodes)
    {
        for (int i = 0; i < AINBFile::ValueTypeCount; i++)
        {
            for (AINBFile::InputEntry Param : Node.InputParameters[i])
            {
                if (Param.NodeIndex >= 0) //Normal Link
                {
                    int Index = 0;
                    int EditorID = -1;
                    for (int j = 0; j < AINBFile::ValueTypeCount; j++)
                    {
                        for (AINBFile::OutputEntry& Entry : this->m_File.Nodes[Param.NodeIndex].OutputParameters[j])
                        {
                            if (Index == Param.ParameterIndex)
                            {
                                EditorID = Entry.EditorID;
                                break;
                            }
                            Index++;
                        }
                    }
                    //std::cout << "Normal Link from " << Param.Name << " to " << EditorID << "\n";
                    ImNodes::Link(Param.EditorID + 1, EditorID, Param.EditorID);
                }
                else //Precondition Link
                {
                    //std::cout << "Precondition Link from " << Param.Name << " to " << Param.EditorID + 2 << "\n";
                    //ImNodes::Link(Param.EditorID + 3, Param.EditorID + 2, Param.EditorID); 
                }
            }
        }
    }

    ImNodes::EndNodeEditor();

    int NumSelectedNodes = ImNodes::NumSelectedNodes();
    if (NumSelectedNodes > 0 && WantDeleteSelected)
    {
        std::vector<int> SelectedNodeIDs(NumSelectedNodes);
        ImNodes::GetSelectedNodes(SelectedNodeIDs.data());

        for (int NodeID : SelectedNodeIDs)
        {
            AINBFile::Node Node;

            for (AINBFile::Node NodeDel : this->m_File.Nodes)
            {
                if (NodeDel.EditorID == NodeID)
                {
                    Node = NodeDel;
                    break;
                }
            }

            std::vector<AINBFile::Node>::iterator Iter;
            for (Iter = this->m_File.Nodes.begin(); Iter != this->m_File.Nodes.end(); ) {

                for (int i = 0; i < AINBFile::ValueTypeCount; i++)
                {
                    for (AINBFile::InputEntry& Param : Iter->InputParameters[i])
                    {
                        if (Param.NodeIndex == Node.NodeIndex) //Normal Link
                        {
                            Param.NodeIndex = -1;
                            Param.ParameterIndex = -1;
                        }
                    }
                }

                if (Iter->EditorID == NodeID)
                    Iter = this->m_File.Nodes.erase(Iter);
                else
                    Iter++;
            }
        }

        //Rebuilding Indexes

        std::map<int, int> IndexMap;

        for (int i = 0; i < this->m_File.Nodes.size(); i++)
        {
            IndexMap.insert({ this->m_File.Nodes[i].NodeIndex, i });
            this->m_File.Nodes[i].NodeIndex = i;
        }
        for (AINBFile::Node& Node : this->m_File.Nodes)
        {
            for (int i = 0; i < AINBFile::LinkedNodeTypeCount; i++)
            {
                for (AINBFile::LinkedNodeInfo& Info : Node.LinkedNodes[i])
                {
                    Info.NodeIndex = IndexMap[Info.NodeIndex];
                }
            }
            for (int i = 0; i < AINBFile::ValueTypeCount; i++)
            {
                for (AINBFile::InputEntry& Param : Node.InputParameters[i])
                {
                    if (Param.NodeIndex != -1) Param.NodeIndex = IndexMap[Param.NodeIndex];
                }
            }
        }
    }

    int NumSelectedLink = ImNodes::NumSelectedLinks();
    if (NumSelectedLink > 0 && WantDeleteSelected)
    {
        std::vector<int> SelectedLinkIDs(NumSelectedLink);
        ImNodes::GetSelectedLinks(SelectedLinkIDs.data());

        for (int LinkID : SelectedLinkIDs)
        {
            for (AINBFile::Node& Node : this->m_File.Nodes)
            {
                for (int i = 0; i < AINBFile::ValueTypeCount; i++)
                {
                    for (AINBFile::InputEntry& Param : Node.InputParameters[i])
                    {
                        if (LinkID == Param.EditorID + 1)
                        {
                            Param.NodeIndex = -1;
                            Param.ParameterIndex = -1;
                            break;
                        }
                    }
                }
            }
        }
    }

    {
        int StartID = 0;
        int EndID = 0;

        if (ImNodes::IsLinkCreated(&StartID, &EndID))
        {
            int DestNodeIndex = 0;
            int DestParamIndex = 0;
            bool Found = false;
            for (AINBFile::Node& Node : this->m_File.Nodes)
            {
                for (int i = 0; i < this->m_File.ValueTypeCount; i++)
                {
                    for (AINBFile::OutputEntry& Param : Node.OutputParameters[i])
                    {
                        if (Param.EditorID == StartID)
                        {
                            Found = true;
                            break;
                        }
                        DestParamIndex++;
                    }
                    if (Found) break;
                }
                if (Found) break;
                DestParamIndex = 0;
                DestNodeIndex++;
            }

            for (AINBFile::Node& Node : this->m_File.Nodes)
            {
                for (int i = 0; i < this->m_File.ValueTypeCount; i++)
                {
                    for (AINBFile::InputEntry& Param : Node.InputParameters[i])
                    {
                        if (Param.EditorID == EndID)
                        {
                            Param.NodeIndex = DestNodeIndex;
                            Param.ParameterIndex = DestParamIndex;

                            /*
                            for (int LinkType = 0; LinkType < AINBFile::LinkedNodeTypeCount; LinkType++)
                            {
                                for (AINBFile::LinkedNodeInfo& Info : Node.LinkedNodes[LinkType])
                                {
                                    Info.NodeIndex = DestNodeIndex;
                                    Info.Parameter = Param.Name;
                                }
                            }
                            */

                            break;
                        }
                    }
                }
            }
        }
    }

    /*
    {
        int EditorID;
        if (ImNodes::IsLinkDestroyed(&EditorID))
        {
            for (AINBFile::Node& Node : this->m_File.Nodes)
            {
                for (int i = 0; i < AINBFile::ValueTypeCount; i++)
                {
                    for (AINBFile::InputEntry& Param : Node.InputParameters[i])
                    {
                        if (EditorID == Param.EditorID + 1)
                        {
                            Param.NodeIndex = -1;
                            Param.ParameterIndex = -1;
                            break;
                        }
                    }
                }
            }
        }
    }
    */
}

void AINBEditor::Initialize()
{
    ImNodesStyle& style = ImNodes::GetStyle();
    style.Flags |= ImNodesStyleFlags_GridLinesPrimary | ImNodesStyleFlags_GridSnapping;
}