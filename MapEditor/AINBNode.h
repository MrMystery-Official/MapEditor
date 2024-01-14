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
    struct FlowLink {
        ed::LinkId LinkID;
        ed::PinId FlowFromPinID;
        AINBFile::LinkedNodeInfo* NodeLink;
    };
    struct ParamLink {
        ed::LinkId LinkID;
        AINBFile::ValueType InputType;
        int InputNodeIdx;
        int InputParameterIdx;
        ed::PinId InputPinID;
        AINBFile::InputEntry* InputParam = nullptr;
        int InputParamIndex;
    };
    // Information for node drawing
    struct AuxInfo {
        uint32_t NodeIdx;
        ImVec2 Pos;
        std::unordered_map<std::string, ImVec2> ExtraNodePos;
    };

    static uint32_t NextID;

    std::vector<ed::PinId> ExtraPins;
    std::vector<FlowLink> FlowLinks;
    std::vector<ParamLink> ParamLinks;

    std::unordered_map<std::string, ed::PinId> NameToPinID;
    std::unordered_map<int, ed::PinId> IdxToID[3];

    ed::PinId FlowPinID;

    AINBImGuiNode(AINBFile::Node* Node);

    void DrawLinks(std::vector<AINBImGuiNode>& Nodes);
    void Draw();
    void UpdateLink(AINBFile::InputEntry& Param, int Index);

    ed::NodeId GetNodeID() { return NodeID; }
    AINBFile::Node& GetNode() { return *Node; }

    AuxInfo GetAuxInfo();
    void LoadAuxInfo(AuxInfo& auxInfo);

private:
    AINBFile::Node* Node;

    int FrameWidth;
    ImVec2 HeaderMin;
    ImVec2 HeaderMax;

    ed::NodeId NodeID;
    std::vector<int> InputPins;
    std::vector<int> OutputPins;
    std::vector<int> ImmediatePins;

    int OutputIdxOffset[AINBFile::ValueTypeCount];

    ImVec2 IconSize = ImVec2(10, 10);
    int MinImmTextboxWidth = 150;

    ed::NodeId static MakeNodeID() { return ++NextID; }
    ed::PinId static MakePinID() { return ++NextID; }
    ed::LinkId static MakeLinkID() { return ++NextID; }

    void DrawPinIcon(ed::PinId ID, bool IsOutput);
    void DrawPinTextCommon(std::string& Name);
    void DrawInputPin(AINBFile::InputEntry& Param, ed::PinId ID);
    void DrawOutputPin(AINBFile::OutputEntry& Param, ed::PinId ID);
    void DrawExtraPins();

    void PreparePinIDs();
    void CalculateFrameWidth();
    static ImColor GetNodeHeaderColor(AINBFile::NodeTypes Type);
    void PrepareTextAlignRight(std::string Str, int ExtraMargin = 0);
};