#pragma once

#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include "BinaryVectorReader.h"
#include "Util.h"
#include "Vector3F.h"
#include "BinaryVectorWriter.h"
#include <variant>
#include <map>

class AINBFile {
public:

	using AINBValue = std::variant<uint32_t, bool, float, std::string, Vector3F>; // Same order as ValueType enum

	struct AinbHeader {
		char Magic[4];
		std::string FileName;
		std::string FileCategory;
		uint32_t Version;
		uint32_t FileNameOffset;
		uint32_t CommandCount;
		uint32_t NodeCount;
		uint32_t PreconditionCount;
		uint32_t AttachmentCount;
		uint32_t OutputCount;
		uint32_t GlobalParameterOffset;
		uint32_t StringOffset;
		uint32_t ResolveOffset;
		uint32_t ImmediateOffset;
		uint32_t ResidentUpdateOffset;
		uint32_t IOOffset;
		uint32_t MultiOffset;
		uint32_t AttachmentOffset;
		uint32_t AttachmentIndexOffset;
		uint32_t EXBOffset;
		uint32_t ChildReplacementOffset;
		uint32_t PreconditionOffset;
		uint32_t x50Section;
		uint32_t x54Value;
		uint32_t x58Section;
		uint32_t EmbedAinbOffset;
		uint32_t x64Value;
		uint32_t EntryStringOffset;
		uint32_t x6cSection;
		uint32_t FileHashOffset;
	};

	struct GUIDData
	{
		uint32_t Part1;
		uint16_t Part2;
		uint16_t Part3;
		uint16_t Part4;
		uint8_t Part5[6];
	};

	struct Command {
		std::string Name;
		GUIDData GUID;
		uint16_t LeftNodeIndex;
		uint16_t RightNodeIndex;
	};

	/*


type_standard = ["int", "bool", "float", "string", "vec3f", "userdefined"] # Data type order

type_global = ["string", "int", "float", "bool", "vec3f", "userdefined"] # Data type order (global parameters)

file_category = {"AI" : 0, "Logic" : 1, "Sequence" : 2}

	*/

	enum class ValueType
	{
		Int,
		Bool,
		Float,
		String,
		Vec3f,
		UserDefined,
		_Count
	};
	enum class GlobalType
	{
		String,
		Int,
		Float,
		Bool,
		Vec3f,
		UserDefined,
		_Count
	};
	static const uint32_t ValueTypeCount = static_cast<uint32_t>(ValueType::_Count);
	static const uint32_t GlobalTypeCount = static_cast<uint32_t>(GlobalType::_Count);

	struct GlobalHeaderEntry {
		uint16_t Count;
		uint16_t Index;
		uint16_t Offset;
	};

	struct GlobalFileRef {
		std::string FileName = "";
		uint32_t NameHash;
		uint32_t UnknownHash1;
		uint32_t UnknownHash2;
	};

	struct GlobalEntry {
		uint32_t Index = 0xFFFFFFFF;
		std::string Name;
		std::string Notes;
		AINBValue GlobalValue;
		int GlobalValueType;
		GlobalFileRef FileReference;
	};

	enum class FlagsStruct : uint8_t {
		PulseThreadLocalStorage = 0,
		SetPointerFlagBitZero = 1,
		IsValidUpdate = 2,
		UpdatePostCurrentCommandCalc = 3,
		IsPreconditionNode = 4,
		IsExternalAINB = 5,
		IsResidentNode = 6
	};

	struct ImmediateParameter {
		std::string Name;
		AINBValue Value;
		int ValueType = -1;
		std::string Class;
		std::vector<FlagsStruct> Flags;
		uint16_t GlobalParametersIndex = 0xFFFF;
	};

	struct AttachmentEntry {
		std::string Name;
		uint32_t Offset;
		uint16_t EXBFunctionCount;
		uint16_t EXBIOSize;
		uint32_t NameHash;
		std::vector<std::vector<ImmediateParameter>> Parameters;
		bool IsRemovedAtRuntie = false;
	};

	struct MultiEntry {
		uint16_t NodeIndex;
		uint16_t ParameterIndex;
		std::vector<FlagsStruct> Flags;
		uint16_t GlobalParametersIndex = 0xFFFF;
	};

	struct InputEntry {
		std::string Name;
		std::string Class;
		int16_t NodeIndex;
		int16_t ParameterIndex;
		int16_t MultiCount;
		int16_t MultiIndex = 0xFFFF;
		std::vector<FlagsStruct> Flags;
		uint16_t GlobalParametersIndex = 0xFFFF;
		AINBValue Value;
		int ValueType;
		std::vector<MultiEntry> Sources;

		int EditorID = -1;
	};

	struct OutputEntry {
		std::string Name;
		bool SetPointerFlagsBitZero = false;
		std::string Class;

		int EditorID = -1;
	};

	struct ResidentEntry {
		std::vector<FlagsStruct> Flags;
		std::string String = "";
	};

	struct EntryStringEntry {
		uint32_t NodeIndex;
		std::string MainState;
		std::string State;
	};

	enum class NodeTypes : uint16_t {
		UserDefined = 0,
		Element_S32Selector = 1,
		Element_Sequential = 2,
		Element_Simultaneous = 3,
		Element_F32Selector = 4,
		Element_StringSelector = 5,
		Element_RandomSelector = 6,
		Element_BoolSelector = 7,
		Element_Fork = 8,
		Element_Join = 9,
		Element_Alert = 10,
		Element_Expression = 20,
		Element_ModuleIF_Input_S32 = 100,
		Element_ModuleIF_Input_F32 = 101,
		Element_ModuleIF_Input_Vec3f = 102,
		Element_ModuleIF_Input_String = 103,
		Element_ModuleIF_Input_Bool = 104,
		Element_ModuleIF_Input_Ptr = 105,
		Element_ModuleIF_Output_S32 = 200,
		Element_ModuleIF_Output_F32 = 201,
		Element_ModuleIF_Output_Vec3f = 202,
		Element_ModuleIF_Output_String = 203,
		Element_ModuleIF_Output_Bool = 204,
		Element_ModuleIF_Output_Ptr = 205,
		Element_ModuleIF_Child = 300,
		Element_StateEnd = 400,
		Element_SplitTiming = 500
	};

	enum class LinkedNodeMapping : uint8_t {
		OutputBoolInputFloatInputLink = 0,
		Unused1 = 1,
		StandardLink = 2,
		ResidentUpdateLink = 3,
		StringInputLink = 4,
		IntInputLink = 5,
		Unused2 = 6,
		Unused3 = 7,
		Unused4 = 8,
		Unused5 = 9,
		_Count
	};
	static const uint32_t LinkedNodeTypeCount = static_cast<uint32_t>(LinkedNodeMapping::_Count);

	struct LinkedNodeInfo {
		LinkedNodeMapping Type;
		uint32_t NodeIndex;
		std::string Parameter = "";
		std::string Condition = "";
		std::string Connection;
		std::string ConnectionName = "";
		float ConditionMin = 0.0f;
		float ConditionMax = -1.0f;
		float Probability = 0.0f;
		ResidentEntry UpdateInfo;

		std::string DynamicStateName;
		std::string DynamicStateValue;
		bool IsRemovedAtRuntime = false;
		uint16_t ReplacementNodeIndex = 0xFFFF;
	};

	struct Node {
		uint16_t Type;
		uint16_t NodeIndex;
		uint16_t AttachmentCount;
		std::vector<FlagsStruct> Flags;
		std::string Name;
		uint32_t NameHash;
		uint32_t ParametersOffset;
		uint16_t EXBFunctionCount;
		uint16_t EXBIOSize;
		uint16_t MultiParamCount = 0;
		uint32_t BaseAttachmentIndex;
		uint16_t BasePreconditionNode = 0;
		uint16_t PreconditionCount = 0;
		GUIDData GUID;
		std::vector<uint16_t> PreconditionNodes;
		std::vector<AttachmentEntry> Attachments;
		std::vector<std::vector<ImmediateParameter>> ImmediateParameters;
		std::vector<std::vector<InputEntry>> InputParameters;
		std::vector<std::vector<OutputEntry>> OutputParameters;
		GlobalEntry Input;
		std::vector<LinkedNodeInfo> LinkedNodes[10];

		int EditorID = -1;
	};

	struct ChildReplace {
		uint8_t Type;
		uint16_t NodeIndex;
		uint16_t ChildIndex = -1;
		uint16_t ReplacementIndex = 0xFFFF;
		uint16_t AttachmentIndex = -1;
	};

	struct EmbeddedAinb {
		std::string FilePath;
		std::string FileCategory;
		uint32_t Count;
	};

	AinbHeader Header;
	std::vector<Command> Commands;
	std::vector<GlobalHeaderEntry> GlobalHeader;
	std::vector<std::vector<GlobalEntry>> GlobalParameters;
	std::vector<GlobalFileRef> GlobalReferences;
	uint32_t MaxGlobalIndex = 0;
	std::vector<uint32_t> ImmediateOffsets;
	std::vector<std::vector<ImmediateParameter>> ImmediateParameters;
	std::vector<AttachmentEntry> AttachmentParameters;
	std::vector<uint32_t> IOOffsets[2];
	std::vector<std::vector<InputEntry>> InputParameters;
	std::vector<std::vector<OutputEntry>> OutputParameters;
	std::vector<ResidentEntry> ResidentUpdateArray;
	std::vector<uint16_t> PreconditionNodes;
	std::vector<EntryStringEntry> EntryStrings;
	std::vector<Node> Nodes;
	std::vector<ChildReplace> Replacements;
	std::vector<EmbeddedAinb> EmbeddedAinbArray;
	std::vector<uint32_t> AttachmentArray;
	bool IsReplaced;
	bool Loaded = false;

	static std::string NodeTypeToString(AINBFile::NodeTypes Type);
	static std::string ValueToString(AINBFile::AINBValue Value);
	std::string ReadStringFromStringPool(BinaryVectorReader* Reader, uint32_t Offset);
	GUIDData ReadGUID(BinaryVectorReader* Reader);
	Node& GetBaseNode();
	static std::string StandardTypeToString(int Type);
	std::string FlagToString(FlagsStruct Flag);
	std::string NodeLinkTypeToString(int Mapping);
	AINBFile(std::vector<unsigned char> Bytes);
	AINBFile(std::string FilePath);
	AINBFile() {}
	std::vector<unsigned char> ToBinary();
	void Write(std::string Path);
private:
	std::stringstream Input;
	std::vector<std::string> StringTable;

	AttachmentEntry ReadAttachmentEntry(BinaryVectorReader* Reader);
	InputEntry ReadInputEntry(BinaryVectorReader* Reader, int Type);
	OutputEntry ReadOutputEntry(BinaryVectorReader* Reader, int Type);
};