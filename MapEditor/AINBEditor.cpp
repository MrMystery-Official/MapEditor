#include "AINBEditor.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"

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
    ImGui::TextUnformatted(("Precondition: " + Parameter.Name + " (" + AINBFile::StandardTypeToString(Parameter.ValueType) + ")").c_str());
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginOutputAttribute(Parameter.EditorID + 2);
    //ImGui::Text(AINBFile::AINBValueToString(Node.Value).c_str());

    
    switch (Parameter.ValueType)
    {
    case (int)AINBFile::ValueType::Bool:
        ImGui::Indent(ImGui::CalcTextSize(("Precondition: " + Parameter.Name + " (" + AINBFile::StandardTypeToString(Parameter.ValueType) + ")").c_str()).x - 16.0f);
        ImGui::Checkbox(("##" + Parameter.Name).c_str(), reinterpret_cast<bool*>(&Parameter.Value));
        break;
    case (int)AINBFile::ValueType::Float:
        ImGui::Indent(ImGui::CalcTextSize(("Precondition: " + Parameter.Name + " (" + AINBFile::StandardTypeToString(Parameter.ValueType) + ")").c_str()).x - 200.0f);
        ImGui::InputScalar(("##" + Parameter.Name).c_str(), ImGuiDataType_::ImGuiDataType_Float, &Parameter.Value);
        break;
    case (int)AINBFile::ValueType::Int:
        ImGui::Indent(ImGui::CalcTextSize(("Precondition: " + Parameter.Name + " (" + AINBFile::StandardTypeToString(Parameter.ValueType) + ")").c_str()).x - 200.0f);
        ImGui::PushItemWidth(200.0f);
        ImGui::InputScalar(("##" + Parameter.Name).c_str(), ImGuiDataType_::ImGuiDataType_U32, &Parameter.Value);
        ImGui::PopItemWidth();
        break;
    case (int)AINBFile::ValueType::String:
        ImGui::PushItemWidth(ImGui::CalcTextSize(("Precondition: " + Parameter.Name + " (" + AINBFile::StandardTypeToString(Parameter.ValueType) + ")").c_str()).x);
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

    for (int i = 0; i < AINBFile::ValueTypeCount; i++)
    {
        for (AINBFile::InputEntry& Param : Node->InputParameters[i])
        {
            if (Param.NodeIndex == -1)
            {
                ImNodes::SetNodeGridSpacePos(Param.EditorID + 1, ImVec2(WidthOffset, HeightOffset));
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
                PreconditionMaxWidth = std::max(PreconditionMaxWidth, ImGui::CalcTextSize(("Precondition: " + Parameter.Name + " (" + AINBFile::StandardTypeToString(Parameter.ValueType) + ")").c_str()).x);
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


    WidthOffset += WidthInput + WidthOutput + 114.0f + PreconditionMaxWidth;

    for (AINBFile::Node& NLink : this->m_File.Nodes)
    {
        for (int i = 0; i < this->m_File.ValueTypeCount; i++)
        {
            for (int j = 0; j < NLink.InputParameters[i].size(); j++)
            {
                AINBFile::InputEntry Parameter = NLink.InputParameters[i][j];
                if (Parameter.NodeIndex == Node->NodeIndex)
                {
                    SetNodePos(&NLink, WidthOffset, HeightOffset);
                    HeightOffset += NodeHeight;
                }
            }
        }
    }
}

void AINBEditor::DrawNodeEditor()
{
    ImGui::Begin("AINB Editor");

    bool WantAutoLayout = ImGui::Button("Auto layout");

    if (ImGui::Button("Save"))
    {
        this->m_File.Write("H:/Paul/switchemulator/Zelda TotK/MapEditorV3/Workspace/LogicWriter/Editor.ainb");
    }

    ImNodes::BeginNodeEditor();

    for (AINBFile::Node& Node : this->m_File.Nodes)
    {
        DrawNode(Node);
    }

    for (AINBFile::Node& Node : this->m_File.Nodes)
    {
        for (int i = 0; i < AINBFile::ValueTypeCount; i++)
        {
            for (AINBFile::InputEntry& Param : Node.InputParameters[i])
            {
                if (Param.NodeIndex == -1)
                {
                    DrawPreconditionNode(Param);
                }
            }
        }
    }

    if (WantAutoLayout)
    {
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
                DestNodeIndex++;
                DestParamIndex = 0;
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
                            break;
                        }
                    }
                }
            }
        }
    }

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

    ImGui::End();
}

AINBEditor::AINBEditor(std::string Path)
{
    this->m_File = AINBFile(Path);

    ImNodes::PushAttributeFlag(ImNodesAttributeFlags_EnableLinkDetachWithDragClick);

    ImNodesStyle& style = ImNodes::GetStyle();
    style.Flags |= ImNodesStyleFlags_GridLinesPrimary | ImNodesStyleFlags_GridSnapping;

    int ID = 0;

    for (AINBFile::Node& Node : this->m_File.Nodes)
    {
        Node.EditorID = ID++;
        for (int i = 0; i < AINBFile::ValueTypeCount; i++)
        {
            for (AINBFile::InputEntry& Param : Node.InputParameters[i])
            {
                Param.EditorID = ID++;
                ID += 3;
            }
            for (AINBFile::OutputEntry& Param : Node.OutputParameters[i])
            {
                Param.EditorID = ID++;
            }
        }
    }
}