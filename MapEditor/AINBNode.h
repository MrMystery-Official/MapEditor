#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_stdlib.h>

#include "AINB.h"
#include "imgui_node_editor/imgui_node_editor.h"

namespace ed = ax::NodeEditor;

namespace PinIcons {
    void DrawIcon(ImVec2& Size);
}

class AINBImGuiNode {
public:
    struct NonNodeInput {
        ed::NodeId GenNodeID;
        ed::PinId GenNodePinID;
        ed::PinId OutputPinID;
        ed::LinkId LinkID;
        AINBFile::InputEntry& InputParam;
    };
    struct FlowLink {
        ed::LinkId LinkID;
        ed::PinId FlowFromPinID;
        AINBFile::LinkedNodeInfo NodeLink;
    };
    struct ParamLink {
        ed::LinkId LinkID;
        AINBFile::ValueType InputType;
        int InputNodeIdx;
        int InputParameterIdx;
        ed::PinId InputPinID;
    };
    // Information for node drawing
    struct AuxInfo {
        uint32_t NodeIdx;
        ImVec2 Pos;
        std::unordered_map<std::string, ImVec2> ExtraNodePos;
    };

    AINBImGuiNode(AINBFile::Node& Node);

    void DrawLinks(std::vector<AINBImGuiNode>& Nodes);
    void Draw();

    ed::NodeId GetNodeID() const { return NodeID; }
    const AINBFile::Node& GetNode() const { return Node; }
    const std::vector<NonNodeInput>& GetNonNodeInputs() const { return NonNodeInputs; }

    AuxInfo GetAuxInfo() const;
    void LoadAuxInfo(const AuxInfo& auxInfo);

private:
    AINBFile::Node& Node;

    int FrameWidth;
    ImVec2 HeaderMin;
    ImVec2 HeaderMax;

    ed::NodeId NodeID;
    ed::PinId FlowPinID;
    std::vector<int> InputPins;
    std::vector<int> OutputPins;
    std::vector<int> ImmediatePins;
    std::vector<ed::PinId> ExtraPins;
    std::vector<NonNodeInput> NonNodeInputs;
    std::vector<FlowLink> FlowLinks;
    std::vector<ParamLink> ParamLinks;

    int OutputIdxOffset[AINBFile::ValueTypeCount];

    std::unordered_map<std::string, ed::PinId> NameToPinID;
    std::unordered_map<int, ed::PinId> IdxToID[3];

    ImVec2 IconSize = ImVec2(10, 10);
    const int MinImmTextboxWidth = 150;

    static uint32_t NextID;
    ed::NodeId static MakeNodeID() { return ++NextID; }
    ed::PinId static MakePinID() { return ++NextID; }
    ed::LinkId static MakeLinkID() { return ++NextID; }

    void DrawPinIcon(ed::PinId ID, bool IsOutput);
    void DrawPinTextCommon(const std::string& Name);
    void DrawInputPin(AINBFile::InputEntry& Param, ed::PinId ID);
    void DrawImmediatePin(AINBFile::ImmediateParameter& Param, ed::PinId ID);
    void DrawOutputPin(const AINBFile::OutputEntry& Param, ed::PinId ID);
    void DrawExtraPins();

    void PreparePinIDs();
    void CalculateFrameWidth();
    static ImColor GetNodeHeaderColor(AINBFile::NodeTypes Type);
    void PrepareTextAlignRight(std::string Str, int ExtraMargin = 0);
};