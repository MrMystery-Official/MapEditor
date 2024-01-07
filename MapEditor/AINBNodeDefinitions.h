#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include "AINB.h"

namespace AINBNodeDefinitions
{
	enum class NodeDefinitionCategory : uint8_t
	{
		AI = 0,
		Logic = 1,
		Sequence = 2 //Currently not used
	};

	struct NodeDefinitionInputParameter
	{
		std::string Name;
		std::string Class;
		AINBFile::AINBValue Value;
		AINBFile::ValueType ValueType;
	};

	struct NodeDefinitionOutputParameter {
		std::string Name;
		std::string Class;
		bool SetPointerFlagsBitZero = false;
	};

	struct NodeDefinitionImmediateParameter {
		std::string Name;
		std::string Class;
		AINBFile::ValueType ValueType;
	};

	struct NodeDefinition
	{
		std::string Name;
		uint32_t NameHash;
		uint16_t Type;
		NodeDefinitionCategory Category;
		std::vector<NodeDefinitionImmediateParameter> ImmediateParameters[6];
		std::vector<NodeDefinitionInputParameter> InputParameters[6];
		std::vector<NodeDefinitionOutputParameter> OutputParameters[6];
	};

	extern std::vector<NodeDefinition> NodeDefinitions;

	void Initialize();
	NodeDefinition* GetNodeDefinition(std::string Name);
	void Generate();
};