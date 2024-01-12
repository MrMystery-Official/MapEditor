#include "AINBNode.h"

#include <sstream>

void PinIconTriangle(ImDrawList* drawList, ImVec2& pos, ImVec2& size, ImColor color) {
    ImVec2 p1 = pos + ImVec2(0, 0);
    ImVec2 p2 = pos + ImVec2(size.x, size.y / 2);
    ImVec2 p3 = pos + ImVec2(0, size.y);

    drawList->AddTriangleFilled(p1, p2, p3, color);
}

void PinIcons::DrawIcon(ImVec2& Size) {
    if (ImGui::IsRectVisible(Size)) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 cursorPos = ImGui::GetCursorScreenPos();

        int fontSize = ImGui::GetFontSize();

        ImVec2 trueDrawPos = cursorPos + ImVec2(0, (Size.y > fontSize) ? 0 : (fontSize - Size.y) / 2);

        PinIconTriangle(drawList, trueDrawPos, Size, ImColor(255, 255, 255));
    }

    ImGui::Dummy(Size);
}

uint32_t AINBImGuiNode::NextID = 0;

AINBImGuiNode::AINBImGuiNode(AINBFile::Node& Node) : Node(&Node) {
    PreparePinIDs();
    CalculateFrameWidth();
}

void AINBImGuiNode::PreparePinIDs()
{
    NodeID = MakeNodeID();
    FlowPinID = MakePinID();

    int InputIndex = 0;
    int OutputIndex = 0;
    int ImmediateIndex = 0;

    for (int Type = 0; Type < AINBFile::ValueTypeCount; Type++)
    {
        for (AINBFile::InputEntry& Param : Node->InputParameters[Type])
        {
            ed::PinId PinID = MakePinID();
            IdxToID[0][InputIndex] = PinID;
            NameToPinID[Param.Name] = PinID;
            InputPins.push_back(InputIndex);
            InputIndex++;
            if (Param.NodeIndex == -1)
            {
                NonNodeInputs.push_back(NonNodeInput{
                    .GenNodeID = MakeNodeID(),
                    .GenNodePinID = MakePinID(),
                    .OutputPinID = PinID,
                    .LinkID = MakeLinkID(),
                    .InputParam = &Param,
                    .InputParamIndex = InputIndex - 1
                    });
            }
            else
            {
                ParamLinks.push_back(ParamLink{
                    .LinkID = MakeLinkID(),
                    .InputType = (AINBFile::ValueType)Param.ValueType,
                    .InputNodeIdx = Param.NodeIndex,
                    .InputParameterIdx = Param.ParameterIndex,
                    .InputPinID = PinID,
                    .InputParam = &Param,
                    .InputParamIndex = InputIndex - 1
                    });
            }
        }
        for (AINBFile::OutputEntry& Param : Node->OutputParameters[Type])
        {
            ed::PinId PinID = MakePinID();
            IdxToID[1][OutputIndex] = PinID;
            NameToPinID[Param.Name] = PinID;
            OutputPins.push_back(OutputIndex);
            OutputIndex++;
        }
        for (AINBFile::ImmediateParameter& Param : Node->ImmediateParameters[Type])
        {
            ed::PinId PinID = MakePinID();
            IdxToID[2][ImmediateIndex] = PinID;
            NameToPinID[Param.Name] = PinID;
            ImmediatePins.push_back(ImmediateIndex);
            ImmediateIndex++;
        }
    }

    // Make index offsets so that the parameter indices work correctly
    // outputIdxOffset[t] is the index of the first output parameter of type t
    int Offset = 0;
    for (int i = 0; i < AINBFile::ValueTypeCount; i++)
    {
        OutputIdxOffset[i] = Offset;
        Offset += Node->OutputParameters[i].size();
    }


    // Extra pins / Flow links
    for (int i = 0; i < AINBFile::LinkedNodeTypeCount; i++)
    {
        for (AINBFile::LinkedNodeInfo& NL : Node->LinkedNodes[i]) {
            if (NL.Type != AINBFile::LinkedNodeMapping::StandardLink)
            {
                if (Node->Type == (uint16_t)AINBFile::NodeTypes::UserDefined || NL.Type != AINBFile::LinkedNodeMapping::ResidentUpdateLink)
                {
                    continue;
                }
            }
            ed::PinId pinID = MakePinID();
            ExtraPins.push_back(pinID);
            switch (Node->Type)
            {
            case (uint16_t)AINBFile::NodeTypes::Element_Simultaneous:
            case (uint16_t)AINBFile::NodeTypes::Element_Fork:
                FlowLinks.push_back(FlowLink{ MakeLinkID(), ExtraPins[0], NL });
                break;
            case (uint16_t)AINBFile::NodeTypes::Element_BoolSelector: {
                FlowLinks.push_back(FlowLink{ MakeLinkID(), pinID, NL });
                break;
            }
            default:
                FlowLinks.push_back(FlowLink{ MakeLinkID(), pinID, NL });
                break;
            }
        }
    }
    if (Node->Type == (uint16_t)AINBFile::NodeTypes::Element_Simultaneous)
    {
        ExtraPins.resize(1);
    }
}

void AINBImGuiNode::CalculateFrameWidth()
{
    int ItemSpacingX = ImGui::GetStyle().ItemSpacing.x;
    FrameWidth = 8 * 2 + ImGui::CalcTextSize(Node->Name.c_str()).x + IconSize.x + ItemSpacingX;

    for (int i = 0; i < AINBFile::ValueTypeCount; i++)
    {
        for (AINBFile::InputEntry& Param : Node->InputParameters[i])
        {
            int Size = 8 * 2; // Frame Padding
            Size += ImGui::CalcTextSize(Param.Name.c_str()).x;
            Size += 2 * ItemSpacingX + IconSize.x;
            FrameWidth = std::max(FrameWidth, Size);
        }
        for (AINBFile::OutputEntry& Param : Node->OutputParameters[i])
        {
            int Size = 8 * 2; // Frame Padding
            Size += ImGui::CalcTextSize(Param.Name.c_str()).x;
            Size += 2 * ItemSpacingX + IconSize.x;
            FrameWidth = std::max(FrameWidth, Size);
        }
        for (AINBFile::ImmediateParameter& Param : Node->ImmediateParameters[i])
        {
            int Size = 8 * 2; // Frame Padding
            Size += ImGui::CalcTextSize(Param.Name.c_str()).x;
            Size += ItemSpacingX + IconSize.x;
            Size += ItemSpacingX + ImGui::GetStyle().FramePadding.x * 2;
            switch (Param.ValueType) {
            case (int)AINBFile::ValueType::Int:
            case (int)AINBFile::ValueType::String:
            case (int)AINBFile::ValueType::Float:
                Size += MinImmTextboxWidth;
                break;
            case (int)AINBFile::ValueType::Bool:
                Size += ImGui::GetFrameHeight();
                break;
            case (int)AINBFile::ValueType::Vec3f:
                Size += MinImmTextboxWidth * 3 + ItemSpacingX * 2;
                break;
            default:
                break;
            }

            FrameWidth = std::max(FrameWidth, Size);
        }
    }
}

ImColor AINBImGuiNode::GetNodeHeaderColor(AINBFile::NodeTypes Type) {
    switch (Type)
    {
    case AINBFile::NodeTypes::Element_S32Selector:
    case AINBFile::NodeTypes::Element_F32Selector:
    case AINBFile::NodeTypes::Element_StringSelector:
    case AINBFile::NodeTypes::Element_RandomSelector:
    case AINBFile::NodeTypes::Element_BoolSelector:
        return ImColor(60, 0, 60);
    case AINBFile::NodeTypes::Element_Sequential:
        return ImColor(60, 0, 0);
    case AINBFile::NodeTypes::Element_Simultaneous:
        return ImColor(0, 60, 0);
    case AINBFile::NodeTypes::Element_ModuleIF_Input_S32:
    case AINBFile::NodeTypes::Element_ModuleIF_Input_F32:
    case AINBFile::NodeTypes::Element_ModuleIF_Input_Vec3f:
    case AINBFile::NodeTypes::Element_ModuleIF_Input_String:
    case AINBFile::NodeTypes::Element_ModuleIF_Input_Bool:
    case AINBFile::NodeTypes::Element_ModuleIF_Input_Ptr:
        return ImColor(0, 60, 60);
    case AINBFile::NodeTypes::Element_ModuleIF_Output_S32:
    case AINBFile::NodeTypes::Element_ModuleIF_Output_F32:
    case AINBFile::NodeTypes::Element_ModuleIF_Output_Vec3f:
    case AINBFile::NodeTypes::Element_ModuleIF_Output_String:
    case AINBFile::NodeTypes::Element_ModuleIF_Output_Bool:
    case AINBFile::NodeTypes::Element_ModuleIF_Output_Ptr:
        return ImColor(0, 0, 60);
    case AINBFile::NodeTypes::Element_ModuleIF_Child:
        return ImColor(0, 0, 0);
    default: return ImColor(60, 60, 0);
    }
}

void AINBImGuiNode::PrepareTextAlignRight(std::string Str, int ExtraMargin)
{
    int cursorPosX = HeaderMax.x;
    cursorPosX -= 8 + ImGui::CalcTextSize(Str.c_str()).x + ExtraMargin;
    ImGui::SetCursorPosX(cursorPosX);
}

void AINBImGuiNode::DrawPinIcon(ed::PinId ID, bool IsOutput)
{
    ed::PushStyleVar(ed::StyleVar_PivotAlignment, ImVec2(0.5f, 1.0f));
    ed::PushStyleVar(ed::StyleVar_PivotSize, ImVec2(0, 0));
    ed::BeginPin(ID, IsOutput ? ed::PinKind::Output : ed::PinKind::Input);
    PinIcons::DrawIcon(IconSize);
    ed::EndPin();
    ed::PopStyleVar(2);
}

void AINBImGuiNode::DrawPinTextCommon(std::string& Name)
{
    ImGui::TextUnformatted(Name.c_str());
}

void AINBImGuiNode::DrawInputPin(AINBFile::InputEntry& Param, ed::PinId ID)
{
    DrawPinIcon(ID, false);
    ImGui::SameLine();
    DrawPinTextCommon(Param.Name);
}

void AINBImGuiNode::DrawImmediatePin(AINBFile::ImmediateParameter& Param, ed::PinId ID)
{
    DrawPinIcon(ID, false);
    ImGui::SameLine();
    DrawPinTextCommon(Param.Name);
        ImGui::SameLine();
        switch (Param.ValueType)
        {
        case (int)AINBFile::ValueType::Int:
        {
            ImGui::PushItemWidth(MinImmTextboxWidth);
            ImGui::InputScalar(("##" + Param.Name + std::to_string(ID.Get())).c_str(), ImGuiDataType_U32, reinterpret_cast<uint32_t*>(&Param.Value));
            ImGui::PopItemWidth();
            break;
        }
        case (int)AINBFile::ValueType::Float:
        {
            ImGui::PushItemWidth(MinImmTextboxWidth);
            ImGui::InputScalar(("##" + Param.Name + std::to_string(ID.Get())).c_str(), ImGuiDataType_Float, reinterpret_cast<float*>(&Param.Value));
            ImGui::PopItemWidth();
            break;
        }
        case (int)AINBFile::ValueType::Bool:
            ImGui::Checkbox(("##" + Param.Name + std::to_string(ID.Get())).c_str(), reinterpret_cast<bool*>(&Param.Value));
            break;
        case (int)AINBFile::ValueType::String:
        {
            ImGui::PushItemWidth(MinImmTextboxWidth);
            ImGui::InputText(("##" + Param.Name + std::to_string(ID.Get())).c_str(), reinterpret_cast<std::string*>(&Param.Value));
            ImGui::PopItemWidth();
            break;
        }
        default:
            ImGui::Text("%s", (*reinterpret_cast<std::string*>(&Param.Value)).c_str());
            break;
        }
}


void AINBImGuiNode::DrawOutputPin(AINBFile::OutputEntry& Param, ed::PinId id)
{
    PrepareTextAlignRight(Param.Name, IconSize.x + ImGui::GetStyle().ItemSpacing.x);
    DrawPinTextCommon(Param.Name);
    ImGui::SameLine();
    DrawPinIcon(id, true);
}

std::string MakeTitle(AINBFile::Node& Node, AINBFile::LinkedNodeInfo& Info, int idx, int count)
{
    switch (Node.Type)
    {
    case (uint16_t)AINBFile::NodeTypes::UserDefined:
    case (uint16_t)AINBFile::NodeTypes::Element_BoolSelector:
    case (uint16_t)AINBFile::NodeTypes::Element_SplitTiming:
        return Node.Name;
    case (uint16_t)AINBFile::NodeTypes::Element_Simultaneous:
        return "Control";
    case (uint16_t)AINBFile::NodeTypes::Element_Sequential:
        return "Seq " + std::to_string(idx);
    case (uint16_t)AINBFile::NodeTypes::Element_S32Selector:
    case (uint16_t)AINBFile::NodeTypes::Element_F32Selector:
        if (idx == count - 1) {
            return "Default";
        }
        return "=" + Info.Parameter;
    case (uint16_t)AINBFile::NodeTypes::Element_Fork:
        return "Fork";
    default:
        return "<name unavailable>";
    }
}

void AINBImGuiNode::DrawExtraPins()
{
    for (size_t i = 0; i < FlowLinks.size(); i++)
    {
        FlowLink& FlowLink = FlowLinks[i];
        std::string title = MakeTitle(*Node, FlowLink.NodeLink, i, FlowLinks.size());
        PrepareTextAlignRight(title, IconSize.x + ImGui::GetStyle().ItemSpacing.x);
        ImGui::TextUnformatted(title.c_str());
        ImGui::SameLine();
        DrawPinIcon(ExtraPins[i], true);
        if (Node->Type == (uint16_t)AINBFile::NodeTypes::Element_Simultaneous || Node->Type == (uint16_t)AINBFile::NodeTypes::Element_Fork)
        {
            break; // Simultaneous and Fork only have one pin
        }
    }
}

void AINBImGuiNode::UpdateLink(AINBFile::InputEntry& Param, int Index)
{
    ed::LinkId ID;
    std::vector<AINBImGuiNode::ParamLink>::iterator IterParam;
    std::vector<AINBImGuiNode::NonNodeInput>::iterator IterValue;
    for (IterParam = this->ParamLinks.begin(); IterParam != this->ParamLinks.end(); ) {
        if (IterParam->InputPinID == IdxToID[0][Index])
        {
            ID = IterParam->LinkID;
            IterParam = this->ParamLinks.erase(IterParam);
        }
        else
            ++IterParam;
    }

    for (IterValue = this->NonNodeInputs.begin(); IterValue != this->NonNodeInputs.end(); ) {
        if (NameToPinID[IterValue->InputParam->Name].Get() == NameToPinID[Param.Name].Get())
        {
            ID = IterValue->LinkID;
            IterValue = this->NonNodeInputs.erase(IterValue);
        }
        else
            ++IterValue;
    }

    if (Param.NodeIndex >= 0)
    {
        ParamLinks.push_back(ParamLink{
            .LinkID = ID,
            .InputType = (AINBFile::ValueType)Param.ValueType,
            .InputNodeIdx = Param.NodeIndex,
            .InputParameterIdx = Param.ParameterIndex,
            .InputPinID = IdxToID[0][Index],
            .InputParam = &Param,
            .InputParamIndex = Index
        });
    }
    else
    {
        NonNodeInputs.push_back(NonNodeInput{
            .GenNodeID = MakeNodeID(),
            .GenNodePinID = MakePinID(),
            .OutputPinID = IdxToID[0][Index],
            .LinkID = ID,
            .InputParam = &Param,
            .InputParamIndex = Index
            });
    }
}

void AINBImGuiNode::Draw() {
    ed::PushStyleVar(ed::StyleVar_NodePadding, ImVec4(8, 8, 8, 8));
    ed::BeginNode(NodeID);
    ed::PushStyleVar(ed::StyleVar_PivotAlignment, ImVec2(0.5f, 1.0f));
    ed::PushStyleVar(ed::StyleVar_PivotSize, ImVec2(0, 0));
    ed::BeginPin(FlowPinID, ed::PinKind::Input);
    PinIcons::DrawIcon(IconSize);
    ed::EndPin();
    ed::PopStyleVar(2);

    ImGui::SameLine();
    ImGui::Text("%s", Node->Name.c_str());

    HeaderMin = ImGui::GetItemRectMin() - ImVec2(IconSize.x + ImGui::GetStyle().ItemSpacing.x + 8, 8);
    HeaderMax = ImVec2(HeaderMin.x + FrameWidth, ImGui::GetItemRectMax().y + 8);

    ImGui::Dummy(ImVec2(0, 8));

    // Main content frame
    int InputIndex = 0;
    int OutputIndex = 0;
    int ImmediateIndex = 0;
    for (int Type = 0; Type < AINBFile::ValueTypeCount; Type++)
    {
        for (AINBFile::InputEntry& Param : Node->InputParameters[Type])
        {
            DrawInputPin(Param, IdxToID[0][InputPins[InputIndex]]);
            InputIndex++;
        }
    }
    for (int Type = 0; Type < AINBFile::ValueTypeCount; Type++)
    {
        for (AINBFile::OutputEntry& Param : Node->OutputParameters[Type])
        {
            DrawOutputPin(Param, IdxToID[1][OutputPins[OutputIndex]]);
            OutputIndex++;
        }
    }
    for (int Type = 0; Type < AINBFile::ValueTypeCount; Type++)
    {
        for (AINBFile::ImmediateParameter& Param : Node->ImmediateParameters[Type])
        {
            DrawImmediatePin(Param, IdxToID[2][ImmediatePins[ImmediateIndex]]);
            ImmediateIndex++;
        }
    }
    DrawExtraPins();
    ed::EndNode();
    ed::PopStyleVar();

    if (ImGui::IsItemVisible()) {
        int alpha = ImGui::GetStyle().Alpha;
        ImColor headerColor = GetNodeHeaderColor(static_cast<AINBFile::NodeTypes>(Node->Type));
        headerColor.Value.w = alpha;

        ImDrawList* drawList = ed::GetNodeBackgroundDrawList(NodeID);

        auto borderWidth = ed::GetStyle().NodeBorderWidth;

        HeaderMin.x += borderWidth;
        HeaderMin.y += borderWidth;
        HeaderMax.x -= borderWidth;
        HeaderMax.y -= borderWidth;

        drawList->AddRectFilled(HeaderMin, HeaderMax, headerColor, ed::GetStyle().NodeRounding,
            ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight);

        ImVec2 headerSeparatorLeft = ImVec2(HeaderMin.x - borderWidth / 2, HeaderMax.y - 0.5f);
        ImVec2 headerSeparatorRight = ImVec2(HeaderMax.x, HeaderMax.y - 0.5f);

        drawList->AddLine(headerSeparatorLeft, headerSeparatorRight, ImColor(255, 255, 255, (int)(alpha * 255 / 2)), borderWidth);
    }
}

void AINBImGuiNode::DrawLinks(std::vector<AINBImGuiNode>& nodes) {
    // Draw inputs not connected to a node
    for (NonNodeInput& input : NonNodeInputs) {
        AINBFile::InputEntry& inputParam = *input.InputParam;
        ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(32, 117, 21, 192));
        ed::BeginNode(input.GenNodeID);
        std::string titleStr = inputParam.Name;
        std::string defaultValueStr = "(" + AINBFile::ValueToString(inputParam.Value) + ")";

        ImGui::TextUnformatted(titleStr.c_str());
        int titleSizeX = ImGui::CalcTextSize(titleStr.c_str()).x;
        int defaultValueSizeX = ImGui::CalcTextSize(defaultValueStr.c_str()).x + ImGui::GetStyle().ItemSpacing.x + IconSize.x;
        if (defaultValueSizeX < titleSizeX) {
            int cursorPosX = ImGui::GetCursorPosX();
            ImGui::SetCursorPosX(cursorPosX + titleSizeX - defaultValueSizeX);
        }
        ImGui::TextUnformatted(defaultValueStr.c_str());
        ImGui::SameLine();
        ed::PushStyleVar(ed::StyleVar_PivotAlignment, ImVec2(0.5f, 1.0f));
        ed::PushStyleVar(ed::StyleVar_PivotSize, ImVec2(0, 0));
        ed::BeginPin(input.GenNodePinID, ed::PinKind::Output);
        PinIcons::DrawIcon(IconSize);
        ed::EndPin();
        ed::PopStyleVar(2);
        ed::EndNode();
        ed::PopStyleColor();

        ed::Link(input.LinkID, input.GenNodePinID, input.OutputPinID, ImColor(255, 255, 255));
    }

    // Draw flow links
    for (FlowLink& flowLink : FlowLinks) {
        ed::Link(flowLink.LinkID, flowLink.FlowFromPinID, nodes[flowLink.NodeLink.NodeIndex].FlowPinID, ImColor(255, 255, 255));
    }

    // Draw node inputs
    for (ParamLink& paramLink : ParamLinks) {
        if (paramLink.InputNodeIdx < 0) {
            // TODO: Multi-links
            continue;
        }

        AINBImGuiNode& inputNode = nodes[paramLink.InputNodeIdx];
        ed::PinId outPin = inputNode.IdxToID[1].at(inputNode.OutputPins[inputNode.OutputIdxOffset[static_cast<int>(paramLink.InputType)] + paramLink.InputParameterIdx]);
        ed::Link(paramLink.LinkID, outPin, paramLink.InputPinID, ImColor(140, 140, 40));
    }
}

AINBImGuiNode::AuxInfo AINBImGuiNode::GetAuxInfo() {
    AuxInfo auxInfo;
    auxInfo.NodeIdx = Node->NodeIndex;
    auxInfo.Pos = ed::GetNodePosition(NodeID);
    for (NonNodeInput& input : NonNodeInputs) {
        auxInfo.ExtraNodePos[input.InputParam->Name] = ed::GetNodePosition(input.GenNodeID);
    }
    return auxInfo;
}

void AINBImGuiNode::LoadAuxInfo(AuxInfo& auxInfo) {
    ed::SetNodePosition(NodeID, auxInfo.Pos);
    for (NonNodeInput& input : NonNodeInputs) {
        if (auxInfo.ExtraNodePos.contains(input.InputParam->Name)) {
            ed::SetNodePosition(input.GenNodeID, auxInfo.ExtraNodePos.at(input.InputParam->Name));
        }
    }
}