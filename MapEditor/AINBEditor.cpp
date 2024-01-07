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

void AINBEditor::SetNodePos(AINBFile::Node* Node, int WidthOffset, int HeightOffset)
{
    ImNodes::SetNodeGridSpacePos(Node->EditorID, ImVec2(WidthOffset, HeightOffset));

    int PreconditionHeight = HeightOffset;

    for (int i = 0; i < AINBFile::ValueTypeCount; i++)
    {
        for (AINBFile::InputEntry& Param : Node->InputParameters[i])
        {
            if (Param.NodeIndex == -1)
            {
                PreconditionHeight -= 70.0f;
            }
        }
    }

    int PreconditionHeightMax = PreconditionHeight - 70.0f;

    for (int i = 0; i < AINBFile::ValueTypeCount; i++)
    {
        for (AINBFile::InputEntry& Param : Node->InputParameters[i])
        {
            if (Param.NodeIndex == -1)
            {
                ImNodes::SetNodeGridSpacePos(Param.EditorID + 1, ImVec2(WidthOffset - ImGui::CalcTextSize(("Value: " + Param.Name + " (" + AINBFile::StandardTypeToString(Param.ValueType) + ")").c_str()).x - 40.0f, PreconditionHeight));
                PreconditionHeight += 70.0f;
            }
        }
    }

    float WidthInput = ImGui::CalcTextSize(Node->Name.c_str()).x;
    float WidthOutput = 0;
    float PreconditionMaxWidth = 150.0f;

    float NodeHeight = 100.0f;

    for (int i = 0; i < this->m_File.ValueTypeCount; i++)
    {
        for (int j = 0; j < Node->InputParameters[i].size(); j++)
        {
            AINBFile::InputEntry Parameter = Node->InputParameters[i][j];
            WidthInput = std::max(WidthInput, ImGui::CalcTextSize(Parameter.Name.c_str()).x);
            NodeHeight += ImGui::CalcTextSize(Parameter.Name.c_str()).y;
            if (Parameter.NodeIndex == -1)
            {
                PreconditionMaxWidth = std::max(PreconditionMaxWidth, ImGui::CalcTextSize(("Value: " + Parameter.Name + " (" + AINBFile::StandardTypeToString(Parameter.ValueType) + ")").c_str()).x + 50.0f);
            }
        }
        for (int j = 0; j < Node->ImmediateParameters[i].size(); j++)
        {
            AINBFile::ImmediateParameter& Parameter = Node->ImmediateParameters[i][j];
            NodeHeight += ImGui::CalcTextSize(Parameter.Name.c_str()).y;
            float Size = ImGui::CalcTextSize(Parameter.Name.c_str()).x;
            /*
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
                
            case (uint32_t)AINBFile::ValueType::Vec3f:
                ImGui::InputFloat3(("##" + Parameter.Name).c_str(), reinterpret_cast<Vector3F*>(&Parameter.Value)->GetRawData());
                break;
                
            }
        */
            WidthInput = std::max(WidthInput, Size);
        }
        ImVec2 windowSize = ImNodes::GetNodeDimensions(Node->EditorID);
        for (int j = 0; j < Node->OutputParameters[i].size(); j++)
        {
            AINBFile::OutputEntry Parameter = Node->OutputParameters[i][j];
            WidthOutput = std::max(WidthOutput, ImGui::CalcTextSize(Parameter.Name.c_str()).x);
            NodeHeight += ImGui::CalcTextSize(Parameter.Name.c_str()).y;
        }
    }

    NodeHeight -= PreconditionHeightMax - 20.0f;

    WidthOffset += WidthInput + WidthOutput + 114.0f + PreconditionMaxWidth;

    std::cout << "Node: " << Node->NodeIndex << ", " << Node->Name << std::endl;

    for (AINBFile::Node& NLink : this->m_File.Nodes)
    {
        std::cout << "NODE: " << NLink.NodeIndex << std::endl;
        for (int i = 0; i < this->m_File.ValueTypeCount; i++)
        {
            for (int j = 0; j < NLink.InputParameters[i].size(); j++)
            {
                AINBFile::InputEntry Parameter = NLink.InputParameters[i][j];
                std::cout << "Param Node Index: " << Parameter.NodeIndex << ", Node Index: " << Node->NodeIndex << std::endl;
                if (Parameter.NodeIndex == Node->NodeIndex)
                {
                    SetNodePos(&NLink, WidthOffset, HeightOffset);
                    HeightOffset += NodeHeight;
                }
            }
        }
    }
}

ImGuiPopUp AddAINBNodePopUp("Add node", 500, 210, 1);

bool VectorOfStringGetter(void* data, int n, const char** out_text)
{
    const std::vector<std::string>* v = (std::vector<std::string>*)data;
    *out_text = v->at(n).c_str();
    return true;
}

void AINBEditor::DrawNodeEditor()
{
    if (ImGui::Button("Open"))
    {
        const char* Path = tinyfd_openFileDialog("Open file", Config::GetRomFSFile("Logic").c_str(), 0, nullptr, nullptr, 0);
        if (Path != nullptr) {
            this->m_File = AINBFile(Config::GetRomFSFile("Logic/" + std::string(strrchr(Path, '\\') + 1)));

            this->CurrentID = 0;

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
        }
    }

    bool WantAutoLayout = false;
    bool WantDeleteSelected = false;
    if (this->m_File.Loaded)
    {
        ImGui::SameLine();

        if (ImGui::Button("Save"))
        {
            this->m_File.Write(Config::GetWorkingDirFile("Save/Logic/" + this->m_File.Header.FileName + ".ainb"));
        }

        WantAutoLayout = ImGui::Button("Auto layout");
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
        AINBFile::Node* Entry = &this->m_File.Nodes[0];

        for (AINBFile::Node& Node : this->m_File.Nodes)
        {
            bool Found = false;
            for (int i = 0; i < AINBFile::ValueTypeCount; i++)
            {
                for (AINBFile::InputEntry Param : Node.InputParameters[i])
                {
                    if (Param.NodeIndex != -1)
                    {
                        Found = true;
                        break;
                    }
                }
            }
            if (!Found)
            {
                Entry = &Node;
                break;
            }
        }

        SetNodePos(Entry, 0, 0);
    }

    for (AINBFile::Node& Node : this->m_File.Nodes)
    {
        for (int i = 0; i < AINBFile::ValueTypeCount; i++)
        {
            for (AINBFile::InputEntry Param : Node.InputParameters[i])
            {
                if (Param.NodeIndex != -1) //Normal Link
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
                    ImNodes::Link(Param.EditorID + 1, EditorID, Param.EditorID);
                }
                else //Precondition Link
                {
                    ImNodes::Link(Param.EditorID + 3, Param.EditorID + 2, Param.EditorID);
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