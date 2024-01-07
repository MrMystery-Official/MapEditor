#include "AINB.h"

std::string AINBFile::ReadStringFromStringPool(BinaryVectorReader* Reader, uint32_t Offset) {
	int BaseOffset = Reader->GetPosition();
	Reader->Seek(Header.StringOffset + Offset, BinaryVectorReader::Position::Begin);
	std::string Result;
	char CurrentCharacter = Reader->ReadInt8();
	while (CurrentCharacter != 0x00) {
		Result += CurrentCharacter;
		CurrentCharacter = Reader->ReadInt8();
	}
	Reader->Seek(BaseOffset, BinaryVectorReader::Position::Begin);
	return Result;
}

AINBFile::GUIDData AINBFile::ReadGUID(BinaryVectorReader* Reader) {
	AINBFile::GUIDData Data;
	Data.Part1 = Reader->ReadUInt32();
	Data.Part2 = Reader->ReadUInt16();
	Data.Part3 = Reader->ReadUInt16();
	Data.Part4 = Reader->ReadUInt16();

	Reader->Read(reinterpret_cast<char*>(Data.Part5), 6);

	return Data;
}

AINBFile::AttachmentEntry AINBFile::ReadAttachmentEntry(BinaryVectorReader* Reader) {
	AINBFile::AttachmentEntry Entry;
	Entry.Name = ReadStringFromStringPool(Reader, Reader->ReadUInt32());
	Entry.Offset = Reader->ReadUInt32();
	Entry.EXBFunctionCount = Reader->ReadUInt16();
	Entry.EXBIOSize = Reader->ReadUInt16();
	Entry.Name = Reader->ReadUInt32();
	return Entry;
}

AINBFile::InputEntry AINBFile::ReadInputEntry(BinaryVectorReader* Reader, int Type) {
	AINBFile::InputEntry Entry;
	Entry.Name = ReadStringFromStringPool(Reader, Reader->ReadUInt32());
	if (Type == 5) { //userdefined
		Entry.Class = ReadStringFromStringPool(Reader, Reader->ReadUInt32());
	}
	Entry.NodeIndex = Reader->ReadInt16();
	Entry.ParameterIndex = Reader->ReadInt16();
	if (Entry.NodeIndex <= -100 && Entry.NodeIndex >= -8192) {
		Entry.MultiIndex = -100 - Entry.NodeIndex;
		Entry.MultiCount = Entry.ParameterIndex;
	}
	uint16_t Index = Reader->ReadUInt16();
	uint16_t Flags = Reader->ReadUInt16();
	if (Flags > 0) {
		if (Flags & 0x80) {
			Entry.Flags.push_back(FlagsStruct::PulseThreadLocalStorage);
		}
		if (Flags & 0x100) {
			Entry.Flags.push_back(FlagsStruct::SetPointerFlagBitZero);
		}
		if ((Flags & 0xc200) == 0xc200) {
			std::cout << "ERROR: Tried to use EXB function, which is currently unsupported!" << std::endl;
		}
		if (Flags & 0x8000) {
			Entry.GlobalParametersIndex = Index;
		}
	}

	if (Type == 0) { //int
		Entry.Value = Reader->ReadUInt32();
	}
	if (Type == 1) { //bool
		Entry.Value = Reader->ReadUInt32();
	}
	if (Type == 2) { //float
		Entry.Value = Reader->ReadFloat();
	}
	if (Type == 3) { //string
		Entry.Value = ReadStringFromStringPool(Reader, Reader->ReadUInt32());
	}
	if (Type == 4) { //vec3f
		Entry.Value = Vector3F(Reader->ReadFloat(), Reader->ReadFloat(), Reader->ReadFloat());
	}
	if (Type == 5) { //userdefined
		Entry.Value = std::to_string(Reader->ReadUInt32());
	}
	Entry.ValueType = Type;
	return Entry;
}

AINBFile::OutputEntry AINBFile::ReadOutputEntry(BinaryVectorReader* Reader, int Type) {
	AINBFile::OutputEntry Entry;
	uint32_t Flags = Reader->ReadUInt32();
	Entry.Name = ReadStringFromStringPool(Reader, Flags & 0x3FFFFFFF);
	uint32_t Flag = Flags & 0x80000000;
	if (Flag > 0) {
		Entry.SetPointerFlagsBitZero = true;
	}
	if (Type == 5) {
		Entry.Class = ReadStringFromStringPool(Reader, Reader->ReadUInt32());
	}
	return Entry;
}

AINBFile::AINBFile(std::vector<unsigned char> Bytes) {
	BinaryVectorReader Reader(Bytes);

	//Checking magic, should be AIB
	char magic[4];
	Reader.Read(magic, 3);
	Reader.Seek(1, BinaryVectorReader::Position::Current);
	magic[3] = '\0';
	if (strcmp(magic, "AIB") != 0) {
		std::cout << "ERROR: Expected AIB, got " << magic << std::endl;
		return;
	}
	std::cout << "Magic verified!" << std::endl;

	strcpy_s(Header.Magic, 4, magic);

	Header.Version = Reader.ReadUInt32();

	if (Header.Version != 0x404 && Header.Version != 0x407) {
		std::cout << "ERROR: Artificial Intelligence Node Binary expected Version 0x404(4.4) or 0x407(4.7), but got " << Header.Version << "!" << std::endl;
		return;
	}

	Header.FileNameOffset = Reader.ReadUInt32();
	Header.CommandCount = Reader.ReadUInt32();
	Header.NodeCount = Reader.ReadUInt32();
	Header.PreconditionCount = Reader.ReadUInt32();
	Header.AttachmentCount = Reader.ReadUInt32();
	Header.OutputCount = Reader.ReadUInt32();
	Header.GlobalParameterOffset = Reader.ReadUInt32();
	Header.StringOffset = Reader.ReadUInt32();
	Header.FileName = ReadStringFromStringPool(&Reader, Header.FileNameOffset);

	Header.ResolveOffset = Reader.ReadUInt32();
	Header.ImmediateOffset = Reader.ReadUInt32();
	Header.ResidentUpdateOffset = Reader.ReadUInt32();
	Header.IOOffset = Reader.ReadUInt32();
	Header.MultiOffset = Reader.ReadUInt32();
	Header.AttachmentOffset = Reader.ReadUInt32();
	Header.AttachmentIndexOffset = Reader.ReadUInt32();
	Header.EXBOffset = Reader.ReadUInt32();
	Header.ChildReplacementOffset = Reader.ReadUInt32();
	Header.PreconditionOffset = Reader.ReadUInt32();
	Header.x50Section = Reader.ReadUInt32();
	Header.x54Value = Reader.ReadUInt32();
	Header.x58Section = Reader.ReadUInt32();
	Header.EmbedAinbOffset = Reader.ReadUInt32();
	Header.FileCategory = ReadStringFromStringPool(&Reader, Reader.ReadUInt32());
	Header.x64Value = Reader.ReadUInt32();
	Header.EntryStringOffset = Reader.ReadUInt32();
	Header.x6cSection = Reader.ReadUInt32();
	Header.FileHashOffset = Reader.ReadUInt32();

	/* Commands */
	this->Commands.resize(Header.CommandCount);
	for (int i = 0; i < Header.CommandCount; i++) {
		Command Cmd;
		Cmd.Name = ReadStringFromStringPool(&Reader, Reader.ReadUInt32());
		Cmd.GUID = ReadGUID(&Reader);
		Cmd.LeftNodeIndex = Reader.ReadUInt16();
		Cmd.RightNodeIndex = Reader.ReadUInt16();
		this->Commands[i] = Cmd;
	}
	int CommandEnd = Reader.GetPosition();

	/* Global Parameters */
	Reader.Seek(Header.GlobalParameterOffset, BinaryVectorReader::Position::Begin);
	this->GlobalHeader.resize(6);
	for (int i = 0; i < 6; i++) {
		GlobalHeaderEntry GHeaderEntry;
		GHeaderEntry.Count = Reader.ReadUInt16();
		GHeaderEntry.Index = Reader.ReadUInt16();
		GHeaderEntry.Offset = Reader.ReadUInt16();
		Reader.Seek(2, BinaryVectorReader::Position::Current);
		this->GlobalHeader[i] = GHeaderEntry;
	}
	this->GlobalParameters.resize(this->GlobalHeader.size());
	for (int j = 0; j < this->GlobalHeader.size(); j++) {
		GlobalHeaderEntry Type = this->GlobalHeader[j];
		std::vector<GlobalEntry> Parameters(Type.Count);
		for (int i = 0; i < Type.Count; i++) {
			GlobalEntry GEntry;
			uint32_t BitField = Reader.ReadUInt32();
			bool ValidIndex = (bool)BitField >> 31;
			if (ValidIndex) {
				GEntry.Index = (BitField >> 24) & 0b1111111;
				if (GEntry.Index > MaxGlobalIndex) {
					this->MaxGlobalIndex = GEntry.Index;
				}
			}
			GEntry.Name = ReadStringFromStringPool(&Reader, BitField & 0x3FFFFF);
			GEntry.Notes = ReadStringFromStringPool(&Reader, Reader.ReadUInt32());
			Parameters[i] = GEntry;
		}
		this->GlobalParameters[j] = Parameters;
	}
	int Pos = Reader.GetPosition();
	for (int i = 0; i < this->GlobalParameters.size(); i++) {
		GlobalHeaderEntry Type = this->GlobalHeader[i];
		Reader.Seek(Pos + GlobalHeader[i].Offset, BinaryVectorReader::Position::Begin);
		for (GlobalEntry& Entry : this->GlobalParameters[i]) {

			if (i == 0) { //int
				Entry.GlobalValue = Reader.ReadUInt32();
			}
			if (i == 1) { //bool
				Entry.GlobalValue = Reader.ReadUInt8();
			}
			if (i == 2) { //float
				Entry.GlobalValue = Reader.ReadFloat();
			}
			if (i == 3) { //string
				Entry.GlobalValue = ReadStringFromStringPool(&Reader, Reader.ReadUInt32());
			}
			if (i == 4) { //vec3f
				Entry.GlobalValue = Vector3F(Reader.ReadFloat(), Reader.ReadFloat(), Reader.ReadFloat());
			}
			if (i == 5) { //userdefined
				Entry.GlobalValue = "None";
			}
			Entry.GlobalValueType = i;
		}
	}
	this->GlobalReferences.resize(this->MaxGlobalIndex + 1);
	for (int i = 0; i < this->MaxGlobalIndex + 1; i++) {
		GlobalFileRef FileRef;
		FileRef.FileName = ReadStringFromStringPool(&Reader, Reader.ReadUInt32());
		FileRef.NameHash = Reader.ReadUInt32();
		FileRef.UnknownHash1 = Reader.ReadUInt32();
		FileRef.UnknownHash2 = Reader.ReadUInt32();
		this->GlobalReferences[i] = FileRef;
	}
	for (int i = 0; i < this->GlobalParameters.size(); i++) {
		for (GlobalEntry Entry : this->GlobalParameters[i]) {
			if (Entry.Index != 0xFFFFFFFF) {
				Entry.FileReference = this->GlobalReferences[Entry.Index];
			}
		}
	}
	/*
	std::vector<std::vector<GlobalEntry>> GlobalParametersNew;
	for (const std::vector<GlobalEntry>& InnerVector : this->GlobalParameters) {
		if (!InnerVector.empty()) {
			GlobalParametersNew.push_back(InnerVector);
		}
	}
	this->GlobalParameters = GlobalParametersNew;
	*/

	/* EXB Section */
	if (Header.EXBOffset != 0) {
		std::cout << "WARNING: The Artificial Intelligence Node Binary, called " << Header.FileName << ", contains a EXB Entry. This is currently not supported." << std::endl;
	}

	/* Immediate Parameters */
	Reader.Seek(Header.ImmediateOffset, BinaryVectorReader::Position::Begin);
	this->ImmediateOffsets.resize(6);
	for (int i = 0; i < 6; i++) {
		this->ImmediateOffsets[i] = Reader.ReadUInt32();
	}
	this->ImmediateParameters.resize(6);
	for (int i = 0; i < this->ImmediateOffsets.size(); i++) {
		Reader.Seek(this->ImmediateOffsets[i], BinaryVectorReader::Position::Begin);
		if (i < 5) {
			while (Reader.GetPosition() < this->ImmediateOffsets[i + 1]) {
				ImmediateParameter Parameter;
				Parameter.Name = ReadStringFromStringPool(&Reader, Reader.ReadUInt32());
				if (i == 5) { //userdefined
					Parameter.Class = ReadStringFromStringPool(&Reader, Reader.ReadUInt32());
				}
				uint16_t Index = Reader.ReadUInt16();
				uint16_t Flags = Reader.ReadUInt16();

				if (Flags > 0) {
					if (Flags & 0x80) {
						Parameter.Flags.push_back(FlagsStruct::PulseThreadLocalStorage);
					}
					if (Flags & 0x100) {
						Parameter.Flags.push_back(FlagsStruct::SetPointerFlagBitZero);
					}
					if ((Flags & 0xc200) == 0xc200) {
						std::cout << "ERROR: Tried to use EXB function, which is currently unsupported!" << std::endl;
					}
					if (Flags & 0x8000) {
						Parameter.GlobalParametersIndex = Index;
					}
				}

				if (i == 0) { //int
					Parameter.Value = Reader.ReadUInt32();
				}
				if (i == 1) { //bool
					Parameter.Value = (bool)Reader.ReadUInt32();
				}
				if (i == 2) { //float
					Parameter.Value = Reader.ReadFloat();
				}
				if (i == 3) { //string
					Parameter.Value = ReadStringFromStringPool(&Reader, Reader.ReadUInt32());
				}
				if (i == 4) { //vec3f
					Parameter.Value = Vector3F(Reader.ReadFloat(), Reader.ReadFloat(), Reader.ReadFloat());
				}
				Parameter.ValueType = i;

				this->ImmediateParameters[i].push_back(Parameter);
			}
		}
		else {
			while (Reader.GetPosition() < Header.IOOffset) {
				ImmediateParameter Parameter;

				Parameter.Name = ReadStringFromStringPool(&Reader, Reader.ReadUInt32());
				if (i == 5) { //userdefined
					Parameter.Class = ReadStringFromStringPool(&Reader, Reader.ReadUInt32());
				}
				uint16_t Index = Reader.ReadUInt16();
				uint16_t Flags = Reader.ReadUInt16();

				if (Flags > 0) {
					if (Flags & 0x80) {
						Parameter.Flags.push_back(FlagsStruct::PulseThreadLocalStorage);
					}
					if (Flags & 0x100) {
						Parameter.Flags.push_back(FlagsStruct::SetPointerFlagBitZero);
					}
					if ((Flags & 0xc200) == 0xc200) {
						std::cout << "ERROR: Tried to use EXB function, which is currently unsupported!" << std::endl;
					}
					if (Flags & 0x8000) {
						Parameter.GlobalParametersIndex = Index;
					}
				}

				if (i == 0) { //int
					Parameter.Value = Reader.ReadUInt32();
				}
				if (i == 1) { //bool
					Parameter.Value = (bool)Reader.ReadUInt32();
				}
				if (i == 2) { //float
					Parameter.Value = Reader.ReadFloat();
				}
				if (i == 3) { //string
					Parameter.Value = ReadStringFromStringPool(&Reader, Reader.ReadUInt32());
				}
				if (i == 4) { //vec3f
					Parameter.Value = Vector3F(Reader.ReadFloat(), Reader.ReadFloat(), Reader.ReadFloat());
				}
				Parameter.ValueType = i;

				std::cout << "Set " << i << std::endl;
				this->ImmediateParameters[i].push_back(Parameter);
			}
		}
	}

	/* Attachment Parameters */
	if (Header.AttachmentCount > 0) {
		Reader.Seek(Header.AttachmentOffset, BinaryVectorReader::Position::Begin);
		this->AttachmentParameters[0] = ReadAttachmentEntry(&Reader);
		while (Reader.GetPosition() < this->AttachmentParameters[0].Offset) {
			this->AttachmentParameters.push_back(ReadAttachmentEntry(&Reader));
		}
		for (AttachmentEntry& Param : this->AttachmentParameters) {
			Reader.Seek(Param.Offset + 4, BinaryVectorReader::Position::Begin);
			std::vector<std::vector<ImmediateParameter>> Parameters;
			for (int i = 0; i < 6; i++) {
				uint32_t Index = Reader.ReadUInt32();
				uint32_t Count = Reader.ReadUInt32();
				Parameters[i].resize(Count);
				for (int j = 0; j < Count; j++) {
					Parameters[i][j] = this->ImmediateParameters[i][Index + j];
				}
				Reader.Seek(48, BinaryVectorReader::Position::Current);
			}
			Param.Parameters = Parameters;
		}
		Reader.Seek(Header.AttachmentIndexOffset, BinaryVectorReader::Position::Begin);
		while (Reader.GetPosition() < Header.AttachmentOffset) {
			this->AttachmentArray.push_back(Reader.ReadUInt32());
		}
	}

	/* IO Parameters */
	Reader.Seek(Header.IOOffset, BinaryVectorReader::Position::Begin);
	for (int i = 0; i < 12; i++) {
		if (i % 2 == 0) {
			this->IOOffsets[0].push_back(Reader.ReadUInt32());
		}
		else {
			this->IOOffsets[1].push_back(Reader.ReadUInt32());
		}
	}
	this->InputParameters.resize(6);
	this->OutputParameters.resize(6);
	//IOOffsets[x],  x = 0 -> Input,  x = 1 -> Output
	for (int i = 0; i < 6; i++) {
		while (Reader.GetPosition() < this->IOOffsets[1][i]) {
			this->InputParameters[i].push_back(ReadInputEntry(&Reader, i));
		}
		if (i < 5) {
			while (Reader.GetPosition() < this->IOOffsets[0][i + 1]) {
				this->OutputParameters[i].push_back(ReadOutputEntry(&Reader, i));
			}
		}
		else {
			while (Reader.GetPosition() < Header.MultiOffset) {
				this->OutputParameters[i].push_back(ReadOutputEntry(&Reader, i));
			}
		}
	}

	/* Multi Parameters */
	for (int Type = 0; Type < this->InputParameters.size(); Type++) {
		for (InputEntry& Parameter : this->InputParameters[Type]) {
			if (Parameter.MultiIndex != -1) {
				Reader.Seek(Header.MultiOffset + Parameter.MultiIndex * 8, BinaryVectorReader::Position::Begin);
				Parameter.Sources.resize(Parameter.MultiCount);
				for (int i = 0; i < Parameter.MultiCount; i++) {
					MultiEntry Entry;

					Entry.NodeIndex = Reader.ReadUInt16();
					Entry.ParameterIndex = Reader.ReadUInt16();
					uint16_t Index = Reader.ReadUInt16();
					uint16_t Flags = Reader.ReadUInt16();
					if (Flags > 0) {
						if (Flags & 0x80) {
							Entry.Flags.push_back(FlagsStruct::PulseThreadLocalStorage);
						}
						if (Flags & 0x100) {
							Entry.Flags.push_back(FlagsStruct::SetPointerFlagBitZero);
						}
						if ((Flags & 0xc200) == 0xc200) {
							std::cout << "ERROR: Tried to use EXB function, which is currently unsupported!" << std::endl;
						}
						if (Flags & 0x8000) {
							Entry.GlobalParametersIndex = Index;
						}
					}

					Parameter.Sources[i] = Entry;
				}
			}
		}
	}

	/* Resident Update Array */
	Reader.Seek(Header.ResidentUpdateOffset, BinaryVectorReader::Position::Begin);
	if (Header.ResidentUpdateOffset != Header.PreconditionOffset) {
		std::vector<uint32_t> Offsets;
		Offsets.push_back(Reader.ReadUInt32());
		while (Reader.GetPosition() < Offsets[0]) {
			Offsets.push_back(Reader.ReadUInt32());
		}
		for (uint32_t Offset : Offsets) {
			Reader.Seek(Offset, BinaryVectorReader::Position::Begin);

			ResidentEntry Entry;
			uint32_t Flags = Reader.ReadUInt32();
			if (Flags >> 31) {
				Entry.Flags.push_back(FlagsStruct::UpdatePostCurrentCommandCalc);
			}
			if (Flags & 1) {
				Entry.Flags.push_back(FlagsStruct::IsValidUpdate);
			}
			if (std::find(Entry.Flags.begin(), Entry.Flags.end(), FlagsStruct::IsValidUpdate) == Entry.Flags.end()) {
				Entry.String = ReadStringFromStringPool(&Reader, Reader.ReadUInt32());
			}

			this->ResidentUpdateArray.push_back(Entry);
		}
	}

	/* Precondition Nodes */
	Reader.Seek(Header.PreconditionOffset, BinaryVectorReader::Position::Begin);
	uint32_t End = Header.EXBOffset != 0 ? Header.EXBOffset : Header.EmbedAinbOffset;
	while (Reader.GetPosition() < End) {
		this->PreconditionNodes.push_back(Reader.ReadUInt16());
		Reader.Seek(2, BinaryVectorReader::Position::Current);
	}

	/* Entry String */
	Reader.Seek(Header.EntryStringOffset, BinaryVectorReader::Position::Begin);
	uint32_t Count = Reader.ReadUInt32();
	this->EntryStrings.resize(Count);
	for (int i = 0; i < Count; i++) {
		EntryStringEntry Entry;
		Entry.NodeIndex = Reader.ReadUInt32();
		Entry.MainState = ReadStringFromStringPool(&Reader, Reader.ReadUInt32());
		Entry.State = ReadStringFromStringPool(&Reader, Reader.ReadUInt32());
		this->EntryStrings[i] = Entry;
	}

	/* Nodes - initialize all nodes and assign corresponding parameters */
	Reader.Seek(CommandEnd, BinaryVectorReader::Position::Begin);
	this->Nodes.resize(Header.NodeCount);
	for (int i = 0; i < Header.NodeCount; i++) {
		Node Node;

		Node.Type = Reader.ReadUInt16();
		Node.NodeIndex = Reader.ReadUInt16();
		Node.AttachmentCount = Reader.ReadUInt16();

		uint8_t Flags = Reader.ReadUInt8();

		if (Flags > 0) {
			if (Flags & 0b1) {
				Node.Flags.push_back(FlagsStruct::IsPreconditionNode);
			}
			if (Flags & 0b10) {
				Node.Flags.push_back(FlagsStruct::IsExternalAINB);
			}
			if (Flags & 0b100) {
				Node.Flags.push_back(FlagsStruct::IsResidentNode);
			}
		}

		Reader.Seek(1, BinaryVectorReader::Position::Current);

		Node.Name = ReadStringFromStringPool(&Reader, Reader.ReadUInt32());

		Node.NameHash = Reader.ReadUInt32();

		Reader.Seek(4, BinaryVectorReader::Position::Current);

		Node.ParametersOffset = Reader.ReadUInt32();
		Node.EXBFunctionCount = Reader.ReadUInt16();
		Node.EXBIOSize = Reader.ReadUInt16();
		Node.MultiParamCount = Reader.ReadUInt16();

		Reader.Seek(2, BinaryVectorReader::Position::Current);

		Node.BaseAttachmentIndex = Reader.ReadUInt32();
		Node.BasePreconditionNode = Reader.ReadUInt16();
		Node.PreconditionCount = Reader.ReadUInt16();

		Reader.Seek(4, BinaryVectorReader::Position::Current);

		Node.GUID = ReadGUID(&Reader);

		if (Node.PreconditionCount > 0) {
			for (int i = 0; i < Node.PreconditionCount; i++) {
				Node.PreconditionNodes.push_back(this->PreconditionNodes[Node.BasePreconditionNode] + i);
			}
		}

		if (Node.AttachmentCount > 0) {
			for (int i = 0; i < Node.AttachmentCount; i++) {
				Node.Attachments.push_back(this->AttachmentParameters[this->AttachmentArray[Node.BaseAttachmentIndex + i]]);
			}
		}

		int NextNodeOffset = Reader.GetPosition();

		std::vector<std::vector<ImmediateParameter>> LocalImmediateParameters(6);
		Reader.Seek(Node.ParametersOffset, BinaryVectorReader::Position::Begin);
		for (int i = 0; i < 6; i++) {
			uint32_t Index = Reader.ReadUInt32();
			uint32_t Count = Reader.ReadUInt32();

			for (int j = 0; j < Count; j++) {
				LocalImmediateParameters[i].push_back(this->ImmediateParameters[i][Index + j]);
			}
		}
		if (LocalImmediateParameters.size() > 0) {
			Node.ImmediateParameters = LocalImmediateParameters;
		}
		std::vector<std::vector<InputEntry>> LocalInputParameters(6);
		std::vector<std::vector<OutputEntry>> LocalOutputParameters(6);
		for (int i = 0; i < 6; i++) {
			uint32_t Index = Reader.ReadUInt32();
			uint32_t Count = Reader.ReadUInt32();
			for (int j = 0; j < Count; j++) {
				LocalInputParameters[i].push_back(this->InputParameters[i][Index + j]);
			}

			Index = Reader.ReadUInt32();
			Count = Reader.ReadUInt32();
			for (int j = 0; j < Count; j++) {
				LocalOutputParameters[i].push_back(this->OutputParameters[i][Index + j]);
			}
		}
		if (LocalInputParameters.size() > 0) {
			Node.InputParameters = LocalInputParameters;
		}
		if (LocalOutputParameters.size() > 0) {
			Node.OutputParameters = LocalOutputParameters;
		}

		std::vector<uint8_t> Counts;
		std::vector<uint8_t> Indices;
		for (int i = 0; i < 10; i++) {
			Counts.push_back(Reader.ReadUInt8());
			Indices.push_back(Reader.ReadUInt8());
		}

		int Start = Reader.GetPosition();
		if (Counts.size() > 0) {
			for (int i = 0; i < 10; i++) {
				Reader.Seek(Start + Indices[i] * 4, BinaryVectorReader::Position::Begin);
				std::vector<uint32_t> Offsets;
				for (int j = 0; j < Counts[i]; j++) {
					Offsets.push_back(Reader.ReadUInt32());
				}
				for (uint32_t Offset : Offsets) {
					Reader.Seek(Offset, BinaryVectorReader::Position::Begin);
					LinkedNodeInfo Info;
					Info.NodeIndex = Reader.ReadUInt32();
					if (i == 0 || i == 4 || i == 5) {
						Info.Parameter = ReadStringFromStringPool(&Reader, Reader.ReadUInt32());
					}
					if (i == 2) {
						std::string Ref = ReadStringFromStringPool(&Reader, Reader.ReadUInt32());
						if (Ref != "") {
							if (Node.Type == (uint16_t)NodeTypes::Element_BoolSelector) {
								Info.Condition = Ref;
							}
							else {
								Info.ConnectionName = Ref;
							}
						}
						else
						{
							int OffsetIndex = 0;
							for (uint32_t Offset2 : Offsets) {
								if (Offset2 == Offset) {
									break;
								}
								OffsetIndex++;
							}
							bool IsEnd = OffsetIndex == (Offsets.size() - 1);
							if (Node.Type == (uint16_t)NodeTypes::Element_S32Selector) {
								uint16_t Index = Reader.ReadUInt16();
								bool Flag = Reader.ReadUInt16() >> 15; //Is Valid Index
								if (Flag) {
									Node.Input = this->GlobalParameters[0][Index];
								}
								if (IsEnd) {
									Info.Condition = "Default";
								}
								else {
									Info.Condition = std::to_string(Reader.ReadInt32());
								}
							}
							else if (Node.Type == (uint16_t)NodeTypes::Element_F32Selector) {
								uint16_t Index = Reader.ReadUInt16();
								bool Flag = Reader.ReadUInt16() >> 15; //Is Valid Index
								if (Flag) {
									Node.Input = this->GlobalParameters[2][Index];
								}
								if (!IsEnd) {
									Info.ConditionMin = Reader.ReadFloat();
									Reader.Seek(4, BinaryVectorReader::Position::Current);
									Info.ConditionMax = Reader.ReadFloat();
								}
								else {
									Info.DynamicStateName = ReadStringFromStringPool(&Reader, Reader.ReadUInt32());
									Info.DynamicStateValue = "Default";
								}
							}
							else if (Node.Type == (uint16_t)NodeTypes::Element_StringSelector) {
								uint16_t Index = Reader.ReadUInt16();
								bool Flag = Reader.ReadUInt16() >> 15; //Is Valid Index
								if (Flag) {
									Node.Input = this->GlobalParameters[3][Index];
								}
								if (IsEnd) {
									Info.DynamicStateName = ReadStringFromStringPool(&Reader, Reader.ReadUInt32());
									Info.DynamicStateValue = "Default";
								}
								else {
									Info.Condition = ReadStringFromStringPool(&Reader, Reader.ReadUInt32());
								}
							}
							else if (Node.Type == (uint16_t)NodeTypes::Element_RandomSelector) {
								Info.Probability = Reader.ReadFloat();
							}
							else {
								Info.ConnectionName = Ref;
							}
						}
					}
					if (i == 3) {
						Info.UpdateInfo = this->ResidentUpdateArray[Reader.ReadUInt32()];
					}
					Node.LinkedNodes[i].push_back(Info);
				}
			}
		}

		Reader.Seek(NextNodeOffset, BinaryVectorReader::Position::Begin);
		this->Nodes[i] = Node;
	}

	/* Child replacement */
	Reader.Seek(Header.ChildReplacementOffset, BinaryVectorReader::Position::Begin);
	this->IsReplaced = Reader.ReadUInt8();
	Reader.Seek(1, BinaryVectorReader::Position::Current);
	Count = Reader.ReadUInt16();
	int16_t NodeCount = Reader.ReadInt16();
	int16_t AttachmentCount = Reader.ReadInt16();
	this->Replacements.resize(Count);
	for (int i = 0; i < Count; i++) {
		ChildReplace Replace;
		Replace.Type = Reader.ReadUInt8();
		Reader.Seek(1, BinaryVectorReader::Position::Current);
		Replace.NodeIndex = Reader.ReadUInt16();
		if (Replace.Type == 0 || Replace.Type == 1) {
			Replace.ChildIndex = Reader.ReadUInt16();
			if (Replace.Type == 1) {
				Replace.ReplacementIndex = Reader.ReadUInt16();
			}
			else {
				Reader.Seek(2, BinaryVectorReader::Position::Current);
			}
		}
		if (Replace.Type == 2) {
			Replace.AttachmentIndex = Reader.ReadUInt16();
			Reader.Seek(2, BinaryVectorReader::Position::Current);
		}
		this->Replacements[i] = Replace;
	}
	if (this->Replacements.size() > 0) {
		for (ChildReplace& Replacement : this->Replacements) {
			if (Replacement.Type == 0) {
				for (int Type = 0; Type < 10; Type++) {
					for (LinkedNodeInfo& LinkedNode : this->Nodes[Replacement.NodeIndex].LinkedNodes[Type]) {
						if (Replacement.ChildIndex == 0) {
							LinkedNode.IsRemovedAtRuntime = true;
						}
					}
				}
			}
			if (Replacement.Type == 1) {
				int i = 0;
				for (int Type = 0; Type < 10; Type++) {
					for (LinkedNodeInfo& LinkedNode : this->Nodes[Replacement.NodeIndex].LinkedNodes[Type]) {
						if (Replacement.ChildIndex == i) {
							LinkedNode.ReplacementNodeIndex = Replacement.ReplacementIndex;
						}
						i++;
					}
				}
			}
			if (Replacement.Type == 2) {
				this->Nodes[Replacement.NodeIndex].Attachments[Replacement.AttachmentIndex].IsRemovedAtRuntie = true;
			}
		}
	}

	/* Embedded AINB */
	Reader.Seek(Header.EmbedAinbOffset, BinaryVectorReader::Position::Begin);
	Count = Reader.ReadUInt32();
	this->EmbeddedAinbArray.resize(Count);
	for (int i = 0; i < Count; i++) {
		EmbeddedAinb Entry;
		Entry.FilePath = ReadStringFromStringPool(&Reader, Reader.ReadUInt32());
		Entry.FileCategory = ReadStringFromStringPool(&Reader, Reader.ReadUInt32());
		Entry.Count = Reader.ReadUInt32();
		this->EmbeddedAinbArray[i] = Entry;
	}
}

AINBFile::Node& AINBFile::GetBaseNode() {
	for (AINBFile::Node& Node : this->Nodes) {
		bool Base = true;
		for (std::vector<AINBFile::LinkedNodeInfo> Type : Node.LinkedNodes) {
			if (!Type.empty()) {
				Base = false;
				break;
			}
		}
		if (Base)
			return Node;
	}
	return this->Nodes[0];
}

AINBFile::AINBFile(std::string FilePath) {
	std::ifstream File(FilePath, std::ios::binary);

	if (!File.eof() && !File.fail())
	{
		File.seekg(0, std::ios_base::end);
		std::streampos FileSize = File.tellg();

		std::vector<unsigned char> Bytes(FileSize);

		File.seekg(0, std::ios_base::beg);
		File.read(reinterpret_cast<char*>(Bytes.data()), FileSize);

		this->AINBFile::AINBFile(Bytes);

		File.close();
	}
	else
	{
		std::cerr << "Could not open file \"" << FilePath << "\"!\n";
	}
}

std::string AINBFile::StandardTypeToString(int Type) {
	switch (Type) {
	case 0:
		return "int";
	case 1:
		return "bool";
	case 2:
		return "float";
	case 3:
		return "string";
	case 4:
		return "vec3f";
	case 5:
		return "userdefined";
	default:
		return "Invalid Type";
	}
}

std::string AINBFile::FlagToString(AINBFile::FlagsStruct Flag) {
	switch ((int)Flag) {
	case 0:
		return "Pulse Thread Local Storage";
	case 1:
		return "Set Pointer Flag Bit Zero";
	case 2:
		return "Is Valid Update";
	case 3:
		return "Update Post Current Command Calc";
	case 4:
		return "Is Precondition Node";
	case 5:
		return "Is External AINB";
	case 6:
		return "Is Resident Node";
	default:
		return "Invalid Flag";
	}
}

std::string AINBFile::NodeLinkTypeToString(int Mapping) {
	switch (Mapping) {
	case 0:
		return "Output/bool Input/float Input Link";
	case 2:
		return "Standard Link";
	case 3:
		return "Resident Update Link";
	case 4:
		return "String Input Link";
	case 5:
		return "Int Input Link";
	default:
		return "Invalid Link Type";
	}
}

int GetOffsetInStringTable(std::string Value, std::vector<std::string>& StringTable)
{
	int Index = -1;
	auto it = std::find(StringTable.begin(), StringTable.end(), Value);

	if (it != StringTable.end())
	{
		Index = it - StringTable.begin();
	}

	if (Index == -1)
	{
		std::cerr << "ERROR: Can not find string \"" << Value << "\" in StringTable of AINB!\n";
		return -1;
	}

	int Offset = 0;

	for (int i = 0; i < Index; i++)
	{
		Offset += StringTable.at(i).length() + 1;
	}

	return Offset;
}

void AddToStringTable(std::string Value, std::vector<std::string>& StringTable)
{
	int Index = -1;
	auto it = std::find(StringTable.begin(), StringTable.end(), Value);

	if (it == StringTable.end())
	{
		StringTable.push_back(Value);
	}
}

std::vector<unsigned char> AINBFile::ToBinary()
{
	BinaryVectorWriter Writer;

	std::vector<std::string> StringTable;

	Writer.WriteBytes("AIB ");
	Writer.WriteByte(0x07);
	Writer.WriteByte(0x04);
	Writer.WriteByte(0x00);
	Writer.WriteByte(0x00);

	AddToStringTable(this->Header.FileName, StringTable);
	Writer.WriteInteger(GetOffsetInStringTable(this->Header.FileName, StringTable), sizeof(uint32_t));

	Writer.WriteInteger(this->Commands.size(), sizeof(uint32_t));
	Writer.WriteInteger(this->Nodes.size(), sizeof(uint32_t));

	uint32_t PreconditionNodeCount = 0;
	for (AINBFile::Node& Node : this->Nodes)
	{
		Node.PreconditionCount = 0;
		for (int i = 0; i < AINBFile::ValueTypeCount; i++)
		{
			for (AINBFile::InputEntry& Param : Node.InputParameters[i])
			{
				if (Param.NodeIndex != -1)
				{
					PreconditionNodeCount++;
					Node.PreconditionCount++;
				}
			}
		}
	}
	Writer.WriteInteger(PreconditionNodeCount, sizeof(uint32_t));

	Writer.WriteInteger(0, sizeof(uint32_t)); //Placeholder

	uint32_t OutputNodeCount = 0;
	for (AINBFile::Node& Node : this->Nodes)
	{
		if ((uint32_t)Node.Type >= 200 && (uint32_t)Node.Type < 300)
			OutputNodeCount++;
	}
	Writer.WriteInteger(OutputNodeCount, sizeof(uint32_t));

	Writer.WriteInteger(116 + 24 * this->Commands.size() + 60 * this->Nodes.size(), sizeof(uint32_t));

	for (int i = 0; i < 11; i++)
	{
		Writer.WriteInteger(4, sizeof(uint32_t)); //Skip offsets until known
	}

	Writer.WriteInteger(0, sizeof(uint64_t)); //Skip
	Writer.WriteInteger(0, sizeof(uint32_t));
	Writer.WriteInteger(0, sizeof(uint32_t));

	AddToStringTable(this->Header.FileCategory, StringTable);
	Writer.WriteInteger(GetOffsetInStringTable(this->Header.FileCategory, StringTable), sizeof(uint32_t));
	uint32_t FileCategoryNum = 0;
	if (this->Header.FileCategory == "Logic") FileCategoryNum = 1;
	if (this->Header.FileCategory == "Sequence") FileCategoryNum = 2;
	Writer.WriteInteger(FileCategoryNum, sizeof(uint32_t));

	Writer.WriteInteger(0, sizeof(uint64_t)); //Skip offset
	Writer.WriteInteger(0, sizeof(uint32_t));

	for (AINBFile::Command Command : this->Commands)
	{
		AddToStringTable(Command.Name, StringTable);
		Writer.WriteInteger(GetOffsetInStringTable(Command.Name, StringTable), sizeof(uint32_t));

		Writer.WriteInteger(Command.GUID.Part1, sizeof(uint32_t));
		Writer.WriteInteger(Command.GUID.Part2, sizeof(uint16_t));
		Writer.WriteInteger(Command.GUID.Part3, sizeof(uint16_t));
		Writer.WriteInteger(Command.GUID.Part4, sizeof(uint16_t));
		Writer.WriteRawUnsafeFixed(reinterpret_cast<char*>(Command.GUID.Part5), 6);

		Writer.WriteInteger(Command.LeftNodeIndex, sizeof(uint16_t));
		Writer.WriteInteger(Command.RightNodeIndex, sizeof(uint16_t));
	}

	//TODO: Reconstruct parameters, etc.

	std::map<uint16_t, uint32_t> AttachCounts;
	std::vector<AINBFile::AttachmentEntry> Attachments;
	std::vector<uint32_t> AttachmentIndices;
	std::vector<AINBFile::MultiEntry> Multis;
	std::map<uint16_t, uint16_t> PreconditionNodes;
	int BasePreconditionOffset = 0;

	AINBFile::GUIDData BaseGUID;
	BaseGUID.Part1 = 0xc29b7607;
	BaseGUID.Part2 = 0xf76a;
	BaseGUID.Part3 = 0x4997;
	BaseGUID.Part4 = 0xf893;
	BaseGUID.Part5[0] = 0xb8;
	BaseGUID.Part5[1] = 0x87;
	BaseGUID.Part5[2] = 0x81;
	BaseGUID.Part5[3] = 0xca;
	BaseGUID.Part5[4] = 0x8c;
	BaseGUID.Part5[5] = 0x65;

	for (AINBFile::Node& Node : this->Nodes)
	{
		Node.GUID = BaseGUID;
		BaseGUID.Part1++;
		Node.BasePreconditionNode = BasePreconditionOffset;
		if (Node.Name.find(".module") != std::string::npos)
		{
			bool FoundExternalAINBFlag = false;
			for (AINBFile::FlagsStruct Flag : Node.Flags)
			{
				if (Flag == AINBFile::FlagsStruct::IsExternalAINB)
				{
					FoundExternalAINBFlag = true;
					break;
				}
			}
			if (!FoundExternalAINBFlag)
			{
				Node.Flags.push_back(AINBFile::FlagsStruct::IsExternalAINB);
			}
		}
		AddToStringTable(Node.Name, StringTable);
		AttachCounts.insert({ Node.NodeIndex, Node.Attachments.size()});
		for (AINBFile::AttachmentEntry Entry : Node.Attachments)
		{
			bool Found = false;

			for (AINBFile::AttachmentEntry Attachment : Attachments)
			{
				if (Attachment.Name == Entry.Name && Attachment.Offset == Entry.Offset)
				{
					Found = true;
					break;
				}
			}

			if (!Found)
			{
				Attachments.push_back(Entry);
			}
		}
		for (AINBFile::AttachmentEntry Entry : Node.Attachments)
		{
			uint32_t Index = 0;
			for (AINBFile::AttachmentEntry Attachment : Attachments)
			{
				if (Attachment.Name == Entry.Name && Attachment.Offset == Entry.Offset)
				{
					break;
				}
				Index++;
			}
			AttachmentIndices.push_back(Index);
		}

		for (int i = 0; i < AINBFile::ValueTypeCount; i++)
		{
			for (AINBFile::InputEntry Param : Node.InputParameters[i])
			{
				if (!Param.Sources.empty())
				{
					for (AINBFile::MultiEntry Entry : Param.Sources)
					{
						bool Found = false;
						for (AINBFile::MultiEntry MultiEntry : Multis)
						{
							if (MultiEntry.NodeIndex == Entry.NodeIndex && MultiEntry.ParameterIndex == Entry.ParameterIndex)
							{
								Found = true;
								break;
							}
						}
						if (!Found) Multis.push_back(Entry);
					}
				}
				if (Param.NodeIndex != -1) //Input param is linked to other node, TODO: Multiple precondition nodes
				{
					PreconditionNodes.insert({ BasePreconditionOffset, Param.NodeIndex });
					BasePreconditionOffset++;

					bool FoundPreconditionFlag = false;
					for (AINBFile::FlagsStruct Flag : Node.Flags)
					{
						if (Flag == AINBFile::FlagsStruct::IsPreconditionNode)
						{
							FoundPreconditionFlag = true;
							break;
						}
					}
					if (!FoundPreconditionFlag)
					{
						Node.Flags.push_back(AINBFile::FlagsStruct::IsPreconditionNode);
					}

					bool FoundPreconditionFlagDest = false;
					for (AINBFile::FlagsStruct Flag : this->Nodes[Param.NodeIndex].Flags)
					{
						if (Flag == AINBFile::FlagsStruct::IsPreconditionNode)
						{
							FoundPreconditionFlagDest = true;
							break;
						}
					}
					if (!FoundPreconditionFlagDest)
					{
						this->Nodes[Param.NodeIndex].Flags.push_back(AINBFile::FlagsStruct::IsPreconditionNode);
					}

					//Node Link
					bool Found = false;
					for (int LinkType = 0; LinkType < AINBFile::LinkedNodeTypeCount; LinkType++)
					{
						for (AINBFile::LinkedNodeInfo& Info : Node.LinkedNodes[LinkType])
						{
							if (Info.NodeIndex == Param.NodeIndex && Info.Parameter == Param.Name)
							{
								Found = true;
								break;
							}
						}
						if (Found) break;
					}
					if (!Found)
					{
						std::cout << "ADD NODE LINK\n";
						AINBFile::LinkedNodeInfo Info;
						Info.NodeIndex = Param.NodeIndex;
						Info.Parameter = Param.Name;
						Node.LinkedNodes[(int)AINBFile::LinkedNodeMapping::OutputBoolInputFloatInputLink].push_back(Info);
					}
				}
			}
		}

		/*
		for (int i = 0; i < Node.PreconditionNodes.size(); i++)
		{
			PreconditionNodes.insert({ Node.BasePreconditionNode + i, Node.PreconditionNodes[i]});
		}
		*/
	}

	bool GlobalParametersEmpty = true;
	for (int Type = 0; Type < AINBFile::GlobalTypeCount; Type++)
	{
		if (!this->GlobalParameters[Type].empty())
		{
			GlobalParametersEmpty = false;
			break;
		}
	}

	Writer.Seek(this->Nodes.size() * 60, BinaryVectorWriter::Position::Current);

	if(!GlobalParametersEmpty) {
		uint16_t Index = 0;
		uint16_t Pos = 0;

		for (int Type = 0; Type < AINBFile::GlobalTypeCount; Type++)
		{
			Writer.WriteInteger(this->GlobalParameters[Type].size(), sizeof(uint16_t));
			Writer.WriteInteger(Index, sizeof(uint16_t));
			Index += this->GlobalParameters[Type].size();
			Writer.WriteInteger(Pos, sizeof(uint16_t));
			if (Type == (int)AINBFile::GlobalType::Vec3f && !this->GlobalParameters[Type].empty())
			{
				Pos += this->GlobalParameters[Type].size() * 12;
			}
			else if (!this->GlobalParameters[Type].empty())
			{
				Pos += this->GlobalParameters[Type].size() * 4;
			}
			Writer.WriteInteger(0, sizeof(uint16_t));
		}

		std::vector<AINBFile::GlobalFileRef> Files;
		for (int Type = 0; Type < AINBFile::GlobalTypeCount; Type++)
		{
			for (AINBFile::GlobalEntry Entry : this->GlobalParameters[Type])
			{
				AddToStringTable(Entry.Name, StringTable);
				uint32_t NameOffset = GetOffsetInStringTable(Entry.Name, StringTable);
				if (Entry.FileReference.FileName != "")
				{
					int FileIndex = -1;

					for (int i = 0; i < Files.size(); i++)
					{
						if (Files[i].FileName == Entry.FileReference.FileName && Files[i].NameHash == Entry.FileReference.NameHash)
						{
							FileIndex = i;
							break;
						}
					}

					if (FileIndex == -1)
					{
						Files.push_back(Entry.FileReference);
						FileIndex = Files.size() - 1;
					}
					NameOffset = NameOffset | (1 << 31);
					NameOffset = NameOffset | (FileIndex << 24);
					Writer.WriteInteger(NameOffset, sizeof(uint32_t));
				}
				else
				{
					NameOffset = NameOffset | (1 << 23);
				}
				Writer.WriteInteger(NameOffset, sizeof(uint32_t));
				AddToStringTable(Entry.Notes, StringTable);
				Writer.WriteInteger(GetOffsetInStringTable(Entry.Notes, StringTable), sizeof(uint32_t));
			}
		}

		int Start = Writer.GetPosition();
		int Size = 0;
		for (int Type = 0; Type < AINBFile::GlobalTypeCount; Type++)
		{
			for (AINBFile::GlobalEntry Entry : this->GlobalParameters[Type])
			{
				if (Type == (int)AINBFile::GlobalType::Int)
				{
					Writer.WriteInteger(*reinterpret_cast<uint32_t*>(&Entry.GlobalValue), sizeof(uint32_t));
					Size += 4;
				}
				if (Type == (int)AINBFile::GlobalType::Float)
				{
					Writer.WriteRawUnsafeFixed(reinterpret_cast<char*>(&Entry.GlobalValue), sizeof(float));
					Size += 4;
				}
				if (Type == (int)AINBFile::GlobalType::Bool)
				{
					Writer.WriteInteger(*reinterpret_cast<bool*>(&Entry.GlobalValue), sizeof(uint32_t));
					Size += 4;
				}
				if (Type == (int)AINBFile::GlobalType::Vec3f)
				{
					Vector3F Vec3f = *reinterpret_cast<Vector3F*>(&Entry.GlobalValue);
					Writer.WriteInteger(Vec3f.GetX(), sizeof(uint32_t));
					Writer.WriteInteger(Vec3f.GetY(), sizeof(uint32_t));
					Writer.WriteInteger(Vec3f.GetZ(), sizeof(uint32_t));
					Size += 12;
				}
				if (Type == (int)AINBFile::GlobalType::String)
				{
					AddToStringTable(*reinterpret_cast<std::string*>(&Entry.GlobalValue), StringTable);
					Writer.WriteInteger(GetOffsetInStringTable(*reinterpret_cast<std::string*>(&Entry.GlobalValue), StringTable), sizeof(uint32_t));
					Size += 4;
				}
			}
		}
		Writer.Seek(Start + Size, BinaryVectorWriter::Position::Begin);
		for (AINBFile::GlobalFileRef File : Files)
		{
			AddToStringTable(File.FileName, StringTable);
			Writer.WriteInteger(GetOffsetInStringTable(File.FileName, StringTable), sizeof(uint32_t));
			Writer.WriteInteger(File.NameHash, sizeof(uint32_t));
			Writer.WriteInteger(File.UnknownHash1, sizeof(uint32_t));
			Writer.WriteInteger(File.UnknownHash2, sizeof(uint32_t));
		}
	}
	else
	{
		Writer.Seek(48, BinaryVectorWriter::Position::Current);
	}

	std::vector<AINBFile::ImmediateParameter> ImmediateParameters[ValueTypeCount];
	std::vector<AINBFile::InputEntry> InputParameters[ValueTypeCount];
	std::vector<AINBFile::OutputEntry> OutputParameters[ValueTypeCount];

	std::map<AINBFile::ValueType, int> ImmediateCurrent;
	ImmediateCurrent.insert({ AINBFile::ValueType::Bool, 0 });
	ImmediateCurrent.insert({ AINBFile::ValueType::Int, 0 });
	ImmediateCurrent.insert({ AINBFile::ValueType::Float, 0 });
	ImmediateCurrent.insert({ AINBFile::ValueType::String, 0 });
	ImmediateCurrent.insert({ AINBFile::ValueType::Vec3f, 0 });
	ImmediateCurrent.insert({ AINBFile::ValueType::UserDefined, 0 });
	std::map<AINBFile::ValueType, int> InputCurrent;
	InputCurrent.insert({ AINBFile::ValueType::Bool, 0 });
	InputCurrent.insert({ AINBFile::ValueType::Int, 0 });
	InputCurrent.insert({ AINBFile::ValueType::Float, 0 });
	InputCurrent.insert({ AINBFile::ValueType::String, 0 });
	InputCurrent.insert({ AINBFile::ValueType::Vec3f, 0 });
	InputCurrent.insert({ AINBFile::ValueType::UserDefined, 0 });
	std::map<AINBFile::ValueType, int> OutputCurrent;
	OutputCurrent.insert({ AINBFile::ValueType::Bool, 0 });
	OutputCurrent.insert({ AINBFile::ValueType::Int, 0 });
	OutputCurrent.insert({ AINBFile::ValueType::Float, 0 });
	OutputCurrent.insert({ AINBFile::ValueType::String, 0 });
	OutputCurrent.insert({ AINBFile::ValueType::Vec3f, 0 });
	OutputCurrent.insert({ AINBFile::ValueType::UserDefined, 0 });
	std::vector<int> Bodies;
	std::vector<AINBFile::ResidentEntry> Residents;

	struct WriterReplacement
	{
		uint8_t Type = -1;
		uint16_t NodeIndex = -1;
		uint16_t Iteration = -1;
		uint16_t ReplacementNodeIndex = -1;
	};

	std::vector<WriterReplacement> Replacements;

	for (AINBFile::Node Node : this->Nodes)
	{
		Bodies.push_back(Writer.GetPosition());

		bool ImmediateParametersEmpty = true;
		bool InputParametersEmpty = true;
		bool OutputParametersEmpty = true;
		bool LinkedNodesEmpty = true;
		for (int Type = 0; Type < AINBFile::ValueTypeCount; Type++)
		{
			if (!Node.ImmediateParameters[Type].empty())
			{
				ImmediateParametersEmpty = false;
			}
			if (!Node.InputParameters[Type].empty())
			{
				InputParametersEmpty = false;
			}
			if (!Node.OutputParameters[Type].empty())
			{
				OutputParametersEmpty = false;
			}
		}

		for (int Type = 0; Type < AINBFile::LinkedNodeTypeCount; Type++)
		{
			if (!Node.LinkedNodes[Type].empty())
			{
				LinkedNodesEmpty = false;
				break;
			}
		}

		std::cout << "Link Empty " << Node.Name << ": " << LinkedNodesEmpty << std::endl;

		if (!ImmediateParametersEmpty)
		{
			for (int Type = 0; Type < AINBFile::ValueTypeCount; Type++)
			{
				if (!Node.ImmediateParameters[Type].empty())
				{
					for (AINBFile::ImmediateParameter Param : Node.ImmediateParameters[Type])
					{
						ImmediateParameters[Type].push_back(Param);
					}
					Writer.WriteInteger(ImmediateParameters[Type].size() - Node.ImmediateParameters[Type].size(), sizeof(uint32_t));
					Writer.WriteInteger(Node.ImmediateParameters[Type].size(), sizeof(uint32_t));
					ImmediateCurrent.find((AINBFile::ValueType)Type)->second = ImmediateParameters[Type].size();
				}
				else
				{
					Writer.WriteInteger(ImmediateCurrent[(AINBFile::ValueType)Type], sizeof(uint32_t));
					Writer.WriteInteger(0, sizeof(uint32_t));
				}
			}
		}
		else
		{
			for (int Type = 0; Type < AINBFile::ValueTypeCount; Type++)
			{
				Writer.WriteInteger(ImmediateCurrent[(AINBFile::ValueType)Type], sizeof(uint32_t));
				Writer.WriteInteger(0, sizeof(uint32_t));
			}
		}

		for (int Type = 0; Type < AINBFile::ValueTypeCount; Type++)
		{
			if (!InputParametersEmpty)
			{
				if (!Node.InputParameters[Type].empty())
				{
					for (AINBFile::InputEntry Param : Node.InputParameters[Type])
					{
						InputParameters[Type].push_back(Param);
					}
					Writer.WriteInteger(InputParameters[Type].size() - Node.InputParameters[Type].size(), sizeof(uint32_t));
					Writer.WriteInteger(Node.InputParameters[Type].size(), sizeof(uint32_t));
					InputCurrent.find((AINBFile::ValueType)Type)->second = InputParameters[Type].size();
				}
				else
				{
					Writer.WriteInteger(InputCurrent[(AINBFile::ValueType)Type], sizeof(uint32_t));
					Writer.WriteInteger(0, sizeof(uint32_t));
				}
			}
			else
			{
				Writer.WriteInteger(InputCurrent[(AINBFile::ValueType)Type], sizeof(uint32_t));
				Writer.WriteInteger(0, sizeof(uint32_t));
			}

			if (!OutputParametersEmpty)
			{
				if (!Node.OutputParameters[Type].empty())
				{
					for (AINBFile::OutputEntry Param : Node.OutputParameters[Type])
					{
						OutputParameters[Type].push_back(Param);
					}
					Writer.WriteInteger(OutputParameters[Type].size() - Node.OutputParameters[Type].size(), sizeof(uint32_t));
					Writer.WriteInteger(Node.OutputParameters[Type].size(), sizeof(uint32_t));
					OutputCurrent.find((AINBFile::ValueType)Type)->second = OutputParameters[Type].size();
				}
				else
				{
					Writer.WriteInteger(OutputCurrent[(AINBFile::ValueType)Type], sizeof(uint32_t));
					Writer.WriteInteger(0, sizeof(uint32_t));
				}
			}
			else
			{
				Writer.WriteInteger(OutputCurrent[(AINBFile::ValueType)Type], sizeof(uint32_t));
				Writer.WriteInteger(0, sizeof(uint32_t));
			}
		}
		if (!LinkedNodesEmpty)
		{
			uint8_t Total = 0;
			for (int Type = 0; Type < AINBFile::LinkedNodeTypeCount; Type++)
			{
				if (!Node.LinkedNodes[Type].empty())
				{
					Writer.WriteInteger(Node.LinkedNodes[Type].size(), sizeof(uint8_t));
					Writer.WriteInteger(Total, sizeof(uint8_t));
					Total += Node.LinkedNodes[Type].size();
				}
				else
				{
					Writer.WriteInteger(0, sizeof(uint8_t));
					Writer.WriteInteger(Total, sizeof(uint8_t));
				}
			}

			uint32_t Start = Writer.GetPosition();
			uint32_t Current = Start + Total * 4;
			int i = 0;
			for (int Type = 0; Type < AINBFile::LinkedNodeTypeCount; Type++)
			{
				if (Type == (int)AINBFile::LinkedNodeMapping::OutputBoolInputFloatInputLink)
				{
					for (AINBFile::LinkedNodeInfo Entry : Node.LinkedNodes[Type])
					{
						Writer.WriteInteger(Current, sizeof(uint32_t));
						uint32_t Pos = Writer.GetPosition();
						Writer.Seek(Current, BinaryVectorWriter::Position::Begin);
						Writer.WriteInteger(Node.NodeIndex, sizeof(uint32_t));
						AddToStringTable(Entry.Parameter, StringTable);
						std::cout << Entry.Parameter << std::endl;
						Writer.WriteInteger(GetOffsetInStringTable(Entry.Parameter, StringTable), sizeof(uint32_t));
						Writer.Seek(Pos, BinaryVectorWriter::Position::Begin);
						bool IsInput = false;
						if (Node.Type == (uint16_t)AINBFile::NodeTypes::Element_Expression ||
							Node.Type == (uint16_t)AINBFile::NodeTypes::Element_S32Selector ||
							Node.Type == (uint16_t)AINBFile::NodeTypes::Element_F32Selector ||
							Node.Type == (uint16_t)AINBFile::NodeTypes::Element_StringSelector ||
							Node.Type == (uint16_t)AINBFile::NodeTypes::Element_RandomSelector ||
							Node.Type == (uint16_t)AINBFile::NodeTypes::Element_BoolSelector)
						{
							for (int i = 0; i < AINBFile::ValueTypeCount; i++)
							{
								for (AINBFile::InputEntry Param : Node.InputParameters[i])
								{
									if (Param.NodeIndex == Entry.NodeIndex)
									{
										IsInput = true;
										break;
									}
								}
							}
						}

						if (IsInput)
						{
							Current += 16;
						}
						else if (Node.Type == (uint16_t)AINBFile::NodeTypes::Element_Expression && !Node.OutputParameters[(int)AINBFile::ValueType::Vec3f].empty())
						{
							for (AINBFile::OutputEntry Parameter : Node.OutputParameters[(int)AINBFile::ValueType::Vec3f])
							{
								if (Entry.Parameter == Parameter.Name)
								{
									Current += 24;
								}
							}
						}
						else
						{
							Current += 8;
						}

						if (Entry.IsRemovedAtRuntime)
						{
							Replacements.push_back({ (uint8_t)0, (uint16_t)Node.NodeIndex, (uint16_t)i, (uint16_t)0 });
						}
						else if (Entry.ReplacementNodeIndex != 0xFFFF)
						{
							Replacements.push_back({ (uint8_t)1, (uint16_t)Node.NodeIndex, (uint16_t)i, (uint16_t)Entry.ReplacementNodeIndex });
						}
						i++;
					}
				}
				else if (Type == (int)AINBFile::LinkedNodeMapping::StandardLink)
				{
					for (AINBFile::LinkedNodeInfo Entry : Node.LinkedNodes[Type])
					{
						if (Node.Type == (int)AINBFile::NodeTypes::Element_F32Selector)
						{
							Writer.WriteInteger(Current, sizeof(uint32_t));
							int Pos = Writer.GetPosition();
							Writer.Seek(Current, BinaryVectorWriter::Position::Begin);
							Writer.WriteInteger(Node.NodeIndex, sizeof(uint32_t));
							AddToStringTable("", StringTable);
							Writer.WriteInteger(GetOffsetInStringTable("", StringTable), sizeof(uint32_t));
							if (Node.Input.Index != 0xFFFFFFFF)
							{
								uint32_t Index = 0;
								for (AINBFile::GlobalEntry GEntry : this->GlobalParameters[(int)AINBFile::GlobalType::Float])
								{
									if (GEntry.Index == Node.Input.Index && GEntry.Name == Node.Input.Name)
									{
										break;
									}
									Index++;
								}
								Writer.WriteInteger(Index | (1 << 31), sizeof(uint32_t));
							}
							else
							{
								Writer.WriteInteger(0, sizeof(uint32_t));
							}
							Writer.WriteRawUnsafeFixed(reinterpret_cast<char*>(&Entry.ConditionMin), sizeof(float));
							Writer.WriteInteger(0, sizeof(uint32_t));
							if (Entry.ConditionMax != -1.0f)
							{
								Writer.WriteRawUnsafeFixed(reinterpret_cast<char*>(&Entry.ConditionMax), sizeof(float));
							}
							else
							{
								Writer.WriteInteger(0, sizeof(uint32_t));
							}
							Writer.Seek(Pos, BinaryVectorWriter::Position::Begin);
							Current += 40;
						}
						else if (Node.Type == (int)AINBFile::NodeTypes::Element_StringSelector ||
							Node.Type == (int)AINBFile::NodeTypes::Element_S32Selector ||
							Node.Type == (int)AINBFile::NodeTypes::Element_RandomSelector)
						{
							Writer.WriteInteger(Current, sizeof(uint32_t));
							int Pos = Writer.GetPosition();
							Writer.Seek(Current, BinaryVectorWriter::Position::Begin);
							Writer.WriteInteger(Node.NodeIndex, sizeof(uint32_t));
							AddToStringTable("", StringTable);
							Writer.WriteInteger(GetOffsetInStringTable("", StringTable), sizeof(uint32_t));
							if (Node.Input.Index != -1)
							{
								uint32_t Index = 0;
								for (AINBFile::GlobalEntry GEntry : this->GlobalParameters[Node.Type])
								{
									if (GEntry.Index == Node.Input.Index && GEntry.Name == Node.Input.Name)
									{
										break;
									}
									Index++;
								}

								Writer.WriteInteger(Index | (1 << 31), sizeof(uint32_t));
							}
							else
							{
								Writer.WriteInteger(0, sizeof(uint32_t));
							}
							if (Entry.Probability != 0.0f)
							{
								Writer.WriteRawUnsafeFixed(reinterpret_cast<char*>(&Entry.Probability), sizeof(float));
							}
							else if (Entry.Condition != "")
							{
								if (Entry.Condition != "Default")
								{
									if (Node.Type == (int)AINBFile::NodeTypes::Element_S32Selector)
									{
										Writer.WriteInteger(Util::StringToNumber<int32_t>(Entry.Condition), sizeof(int32_t));
									}
									else
									{
										AddToStringTable(Entry.Condition, StringTable);
										Writer.WriteInteger(GetOffsetInStringTable(Entry.Condition, StringTable), sizeof(uint32_t));
									}
								}
								else
								{
									Writer.WriteInteger(0, sizeof(uint32_t));
								}
							}
							Writer.Seek(Pos, BinaryVectorWriter::Position::Begin);
							Current += 16;
						}
						else
						{
							Writer.WriteInteger(Current, sizeof(uint32_t));
							int Pos = Writer.GetPosition();
							Writer.Seek(Pos, BinaryVectorWriter::Position::Begin);
							Writer.WriteInteger(Entry.NodeIndex, sizeof(uint32_t));
							if (Entry.ConnectionName != "")
							{
								AddToStringTable(Entry.ConnectionName, StringTable);
								Writer.WriteInteger(GetOffsetInStringTable(Entry.ConnectionName, StringTable), sizeof(uint32_t));
							}
							else if (Entry.Condition != "")
							{
								AddToStringTable(Entry.Condition, StringTable);
								Writer.WriteInteger(GetOffsetInStringTable(Entry.Condition, StringTable), sizeof(uint32_t));
							}
							Writer.Seek(Pos, BinaryVectorWriter::Position::Begin);
							Current += 8;
						}

						if (Entry.IsRemovedAtRuntime)
						{
							Replacements.push_back({ (uint8_t)0, (uint16_t)Node.NodeIndex, (uint16_t)i, (uint16_t)0 });
						}
						else if (Entry.ReplacementNodeIndex != 0xFFFF)
						{
							Replacements.push_back({ (uint8_t)1, (uint16_t)Node.NodeIndex, (uint16_t)i, (uint16_t)Entry.ReplacementNodeIndex });
						}
						i++;
					}
				}
				else if (Type != (int)AINBFile::LinkedNodeMapping::ResidentUpdateLink)
				{
					for (AINBFile::LinkedNodeInfo Entry : Node.LinkedNodes[Type])
					{
						Writer.WriteInteger(Current, sizeof(uint32_t));
						int Pos = Writer.GetPosition();
						Writer.Seek(Pos, BinaryVectorWriter::Position::Begin);
						Writer.WriteInteger(Entry.NodeIndex, sizeof(uint32_t));
						if (Entry.Condition != "")
						{
							AddToStringTable(Entry.Condition, StringTable);
							Writer.WriteInteger(GetOffsetInStringTable(Entry.Condition, StringTable), sizeof(uint32_t));
						}
						else if (Entry.Parameter != "")
						{
							AddToStringTable(Entry.Parameter, StringTable);
							Writer.WriteInteger(GetOffsetInStringTable(Entry.Parameter, StringTable), sizeof(uint32_t));
						}
						else
						{
							AddToStringTable(Entry.ConnectionName, StringTable);
							Writer.WriteInteger(GetOffsetInStringTable(Entry.ConnectionName, StringTable), sizeof(uint32_t));
						}

						if (Node.Input.Index != -1 && (Node.Type == (int)AINBFile::NodeTypes::Element_StringSelector || Node.Type == (int)AINBFile::NodeTypes::Element_S32Selector))
						{
							uint32_t Index = 0;
							for (AINBFile::GlobalEntry GEntry : this->GlobalParameters[Node.Type])
							{
								if (GEntry.Index == Node.Input.Index && GEntry.Name == Node.Input.Name)
								{
									break;
								}
								Index++;
							}

							Writer.WriteInteger(Index | (1 << 31), sizeof(uint32_t));
						}
						Writer.Seek(Pos, BinaryVectorWriter::Position::Begin);

						if (Node.Type == (uint16_t)AINBFile::NodeTypes::Element_Expression ||
							Node.Type == (uint16_t)AINBFile::NodeTypes::Element_S32Selector ||
							Node.Type == (uint16_t)AINBFile::NodeTypes::Element_F32Selector ||
							Node.Type == (uint16_t)AINBFile::NodeTypes::Element_StringSelector ||
							Node.Type == (uint16_t)AINBFile::NodeTypes::Element_RandomSelector ||
							Node.Type == (uint16_t)AINBFile::NodeTypes::Element_BoolSelector)
						{
							Current += 16;
						}
						else
						{
							Current += 8;
						}

						if (Entry.IsRemovedAtRuntime)
						{
							Replacements.push_back({ (uint8_t)0, (uint16_t)Node.NodeIndex, (uint16_t)i, (uint16_t)0 });
						}
						else if (Entry.ReplacementNodeIndex != 0xFFFF)
						{
							Replacements.push_back({ (uint8_t)1, (uint16_t)Node.NodeIndex, (uint16_t)i, (uint16_t)Entry.ReplacementNodeIndex });
						}
						i++;
					}
				}
				else
				{
					for (AINBFile::LinkedNodeInfo Entry : Node.LinkedNodes[Type])
					{
						Writer.WriteInteger(Current, sizeof(uint32_t));
						int Pos = Writer.GetPosition();
						Writer.Seek(Current, BinaryVectorWriter::Position::Begin);
						Writer.WriteInteger(Entry.NodeIndex, sizeof(uint32_t));
						Residents.push_back(Entry.UpdateInfo);
						Writer.WriteInteger(Residents.size() - 1, sizeof(uint32_t));
						Current += 8;
						if (Entry.IsRemovedAtRuntime)
						{
							Replacements.push_back({ (uint8_t)0, (uint16_t)Node.NodeIndex, (uint16_t)i, (uint16_t)0 });
						}
						else if (Entry.ReplacementNodeIndex != 0xFFFF)
						{
							Replacements.push_back({ (uint8_t)1, (uint16_t)Node.NodeIndex, (uint16_t)i, (uint16_t)Entry.ReplacementNodeIndex });
						}
						i++;
					}
				}
			}
			Writer.Seek(Current, BinaryVectorWriter::Position::Begin);
		}
		else
		{
			for (int i = 0; i < 5; i++)
			{
				Writer.WriteInteger(0, sizeof(uint32_t));
			}
		}
	}

	uint32_t AttachmentIndexStart = Writer.GetPosition();
	if (!this->Nodes.empty())
	{
		uint32_t BaseAttach = 0;
		Writer.Seek(116 + 24 * this->Commands.size(), BinaryVectorWriter::Position::Begin);
		for (AINBFile::Node Node : this->Nodes)
		{
			Writer.WriteInteger(Node.Type, sizeof(uint16_t));
			Writer.WriteInteger(Node.NodeIndex, sizeof(uint16_t));
			Writer.WriteInteger(AttachCounts[Node.NodeIndex], sizeof(uint16_t));
			uint8_t Flags = 0;
			for (AINBFile::FlagsStruct Flag : Node.Flags)
			{
				if (Flag == FlagsStruct::IsPreconditionNode)
				{
					Flags = Flags | 1;
				}
				if (Flag == FlagsStruct::IsExternalAINB)
				{
					Flags = Flags | 2;
				}
				if (Flag == FlagsStruct::IsResidentNode)
				{
					Flags = Flags | 4;
				}
			}
			Writer.WriteInteger(Flags, sizeof(uint8_t));
			Writer.WriteInteger(0, 1);
			Writer.WriteInteger(GetOffsetInStringTable(Node.Name, StringTable), sizeof(uint32_t));
			Writer.WriteInteger(Node.NameHash, sizeof(uint32_t)); //TODO: MurMurHash3
			Writer.WriteInteger(0, sizeof(uint32_t));
			Writer.WriteInteger(Bodies[Node.NodeIndex], sizeof(uint32_t));
			Writer.WriteInteger(0, sizeof(uint32_t)); //TODO: EXB shit
			Writer.WriteInteger(Node.MultiParamCount, sizeof(uint16_t)); //TODO: Reconstruct all this shit
			Writer.WriteInteger(0, sizeof(uint16_t));
			Writer.WriteInteger(BaseAttach, sizeof(uint32_t));
			Writer.WriteInteger(Node.BasePreconditionNode, sizeof(uint16_t));
			Writer.WriteInteger(Node.PreconditionCount, sizeof(uint16_t)); //TODO: Also reconstruct this...
			Writer.WriteInteger(0, sizeof(uint32_t)); //Padding
			Writer.WriteInteger(Node.GUID.Part1, sizeof(uint32_t));
			Writer.WriteInteger(Node.GUID.Part2, sizeof(uint16_t));
			Writer.WriteInteger(Node.GUID.Part3, sizeof(uint16_t));
			Writer.WriteInteger(Node.GUID.Part4, sizeof(uint16_t));
			Writer.WriteRawUnsafeFixed(reinterpret_cast<char*>(Node.GUID.Part5), 6);
			BaseAttach += AttachCounts[Node.NodeIndex];
		}
	}

	Writer.Seek(AttachmentIndexStart, BinaryVectorWriter::Position::Begin);
	
	for (uint32_t Entry : AttachmentIndices)
	{
		Writer.WriteInteger(Entry, sizeof(uint32_t));
	}
	uint32_t AttachmentStart = Writer.GetPosition();
	int AttachmentIndex = 0;
	if (!Attachments.empty())
	{
		for (AINBFile::AttachmentEntry Attachment : Attachments)
		{
			AddToStringTable(Attachment.Name, StringTable);
			Writer.WriteInteger(GetOffsetInStringTable(Attachment.Name, StringTable), sizeof(uint32_t));
			Writer.WriteInteger(AttachmentStart + 16 * Attachments.size() + 100 * AttachmentIndex, sizeof(uint32_t));
			Writer.WriteInteger(0, sizeof(uint32_t)); //TODO: Again, EXB...
			Writer.WriteInteger(Attachment.NameHash, sizeof(uint32_t)); //Hash..
			AttachmentIndex++;
		}
		for (AINBFile::AttachmentEntry Attachment : Attachments)
		{
			if (Attachment.Name.find("Debug") != std::string::npos)
			{
				Writer.WriteInteger(1, sizeof(uint32_t));
			}
			else
			{
				Writer.WriteInteger(0, sizeof(uint32_t));
			}
			for (int Type = 0; Type < AINBFile::ValueTypeCount; Type++)
			{
				if (!Attachment.Parameters[Type].empty())
				{
					for (AINBFile::ImmediateParameter Param : Attachment.Parameters[Type])
					{
						ImmediateParameters[Type].push_back(Param);
					}
					Writer.WriteInteger(ImmediateParameters[Type].size() - Attachment.Parameters[Type].size(), sizeof(uint32_t));
					Writer.WriteInteger(Attachment.Parameters[Type].size(), sizeof(uint32_t));
					ImmediateCurrent.find((AINBFile::ValueType)Type)->second = ImmediateParameters[Type].size();
				}
				else
				{
					Writer.WriteInteger(ImmediateCurrent[(AINBFile::ValueType)Type], sizeof(uint32_t));
					Writer.WriteInteger(0, sizeof(uint32_t));
				}
			}
			int Pos = Writer.GetPosition();
			for (int i = 0; i < 6; i++)
			{
				Writer.WriteInteger(0, sizeof(uint32_t));
				Writer.WriteInteger(Pos + 24, sizeof(uint32_t));
			}
		}
	}
	else
	{
		AttachmentStart = Writer.GetPosition();
	}

	uint32_t ImmediateStart = Writer.GetPosition();
	int Current = ImmediateStart + 24;
	bool ImmediateParametersEmpty = true;
	for (int i = 0; i < AINBFile::ValueTypeCount; i++)
	{
		if (!ImmediateParameters[i].empty())
		{
			ImmediateParametersEmpty = false;
			break;
		}
	}
	if (!ImmediateParametersEmpty)
	{
		for (int i = 0; i < AINBFile::ValueTypeCount; i++)
		{
			Writer.WriteInteger(Current, sizeof(uint32_t));
			if (i != (int)AINBFile::ValueType::Vec3f)
			{
				Current += ImmediateParameters[i].size() * 12;
			}
			else
			{
				Current += ImmediateParameters[i].size() * 20;
			}
		}
		for (int Type = 0; Type < AINBFile::ValueTypeCount; Type++)
		{
			for (AINBFile::ImmediateParameter Entry : ImmediateParameters[Type])
			{
				AddToStringTable(Entry.Name, StringTable);
				Writer.WriteInteger(GetOffsetInStringTable(Entry.Name, StringTable), sizeof(uint32_t));
				uint16_t Flags = 0x0;
				if (Type == (int)AINBFile::ValueType::UserDefined)
				{
					AddToStringTable(Entry.Class, StringTable);
					Writer.WriteInteger(GetOffsetInStringTable(Entry.Class, StringTable), sizeof(uint32_t));
				}
				if (Entry.GlobalParametersIndex != 0xFFFF)
				{
					Writer.WriteInteger(Entry.GlobalParametersIndex, sizeof(uint16_t));
					Flags += 0x8000;
				}
				else //TODO: EXB..
				{
					Writer.WriteInteger(0, sizeof(uint16_t));
				}
				if (!Entry.Flags.empty())
				{
					for (AINBFile::FlagsStruct Flag : Entry.Flags)
					{
						if (Flag == AINBFile::FlagsStruct::PulseThreadLocalStorage)
						{
							Flags += 0x80;
						}
						if (Flag == AINBFile::FlagsStruct::SetPointerFlagBitZero)
						{
							Flags += 0x100;
						}
					}
				}
				Writer.WriteInteger(Flags, sizeof(uint16_t));
				if (Type == (int)AINBFile::ValueType::Int)
				{
					Writer.WriteInteger(*reinterpret_cast<int*>(&Entry.Value), sizeof(int32_t));
				}
				else if (Type == (int)AINBFile::ValueType::Bool)
				{
					Writer.WriteInteger(*reinterpret_cast<bool*>(&Entry.Value), sizeof(uint32_t));
				}
				else if (Type == (int)AINBFile::ValueType::Float)
				{
					Writer.WriteRawUnsafeFixed(reinterpret_cast<char*>(&Entry.Value), sizeof(float));
				}
				else if (Type == (int)AINBFile::ValueType::String)
				{
					AddToStringTable(*reinterpret_cast<std::string*>(&Entry.Value), StringTable);
					Writer.WriteInteger(GetOffsetInStringTable(*reinterpret_cast<std::string*>(&Entry.Value), StringTable), sizeof(uint32_t));
				}
				else if (Type == (int)AINBFile::ValueType::Vec3f)
				{
					Vector3F Vec3f = *reinterpret_cast<Vector3F*>(&Entry.Value);
					Writer.WriteRawUnsafeFixed(reinterpret_cast<char*>(&Vec3f.GetRawData()[0]), sizeof(float));
					Writer.WriteRawUnsafeFixed(reinterpret_cast<char*>(&Vec3f.GetRawData()[1]), sizeof(float));
					Writer.WriteRawUnsafeFixed(reinterpret_cast<char*>(&Vec3f.GetRawData()[2]), sizeof(float));
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < 6; i++)
		{
			Writer.WriteInteger(0, sizeof(uint32_t));
		}
	}

	uint32_t IOStart = Writer.GetPosition();
	Current = IOStart + 48;

	bool InputParametersEmpty = true;
	bool OutputParametersEmpty = true;

	for (int i = 0; i < AINBFile::ValueTypeCount; i++)
	{
		if (!InputParameters[i].empty()) InputParametersEmpty = false;
		if (!OutputParameters[i].empty()) OutputParametersEmpty = false;
	}

	if (!InputParametersEmpty && !OutputParametersEmpty)
	{
		for (int Type = 0; Type < AINBFile::ValueTypeCount; Type++)
		{
			Writer.WriteInteger(Current, sizeof(uint32_t));
			if (Type == (int)AINBFile::ValueType::Vec3f)
			{
				Current += InputParameters[Type].size() * 24;
			}
			else if (Type == (int)AINBFile::ValueType::UserDefined)
			{
				Current += InputParameters[Type].size() * 20;
			}
			else
			{
				Current += InputParameters[Type].size() * 16;
			}
			Writer.WriteInteger(Current, sizeof(uint32_t));
			if (Type != (int)AINBFile::ValueType::UserDefined)
			{
				Current += OutputParameters[Type].size() * 4;
			}
			else
			{
				Current += OutputParameters[Type].size() * 8;
			}
		}
		for (int Type = 0; Type < AINBFile::ValueTypeCount; Type++)
		{
			for (AINBFile::InputEntry Entry : InputParameters[Type])
			{
				AddToStringTable(Entry.Name, StringTable);
				Writer.WriteInteger(GetOffsetInStringTable(Entry.Name, StringTable), sizeof(uint32_t));
				uint16_t Flags = 0x0;
				if (Type == (int)AINBFile::ValueType::UserDefined)
				{
					AddToStringTable(Entry.Class, StringTable);
					Writer.WriteInteger(GetOffsetInStringTable(Entry.Class, StringTable), sizeof(uint32_t));
				}
				Writer.WriteInteger(Entry.NodeIndex, sizeof(int16_t));
				Writer.WriteInteger(Entry.ParameterIndex, sizeof(int16_t));
				if (Entry.GlobalParametersIndex != 0xFFFF)
				{
					Writer.WriteInteger(Entry.GlobalParametersIndex, sizeof(uint16_t));
					Flags += 0x8000;
				}
				else //TODO: EXB
				{
					Writer.WriteInteger(0, sizeof(uint16_t));
				}

				if (!Entry.Flags.empty())
				{
					for (AINBFile::FlagsStruct Flag : Entry.Flags)
					{
						if (Flag == AINBFile::FlagsStruct::PulseThreadLocalStorage)
						{
							Flags += 0x80;
						}
						if (Flag == AINBFile::FlagsStruct::SetPointerFlagBitZero)
						{
							Flags += 0x100;
						}
					}
				}
				Writer.WriteInteger(Flags, sizeof(uint16_t));

				if (Type == (int)AINBFile::ValueType::Int)
				{
					Writer.WriteInteger(*reinterpret_cast<int*>(&Entry.Value), sizeof(int32_t));
				}
				else if (Type == (int)AINBFile::ValueType::Bool)
				{
					Writer.WriteInteger(*reinterpret_cast<bool*>(&Entry.Value), sizeof(uint32_t));
				}
				else if (Type == (int)AINBFile::ValueType::Float)
				{
					Writer.WriteRawUnsafeFixed(reinterpret_cast<char*>(&Entry.Value), sizeof(float));
				}
				else if (Type == (int)AINBFile::ValueType::String)
				{
					AddToStringTable(*reinterpret_cast<std::string*>(&Entry.Value), StringTable);
					Writer.WriteInteger(GetOffsetInStringTable(*reinterpret_cast<std::string*>(&Entry.Value), StringTable), sizeof(uint32_t));
				}
				else if (Type == (int)AINBFile::ValueType::Vec3f)
				{
					Vector3F Vec3f = *reinterpret_cast<Vector3F*>(&Entry.Value);
					Writer.WriteRawUnsafeFixed(reinterpret_cast<char*>(&Vec3f.GetRawData()[0]), sizeof(float));
					Writer.WriteRawUnsafeFixed(reinterpret_cast<char*>(&Vec3f.GetRawData()[1]), sizeof(float));
					Writer.WriteRawUnsafeFixed(reinterpret_cast<char*>(&Vec3f.GetRawData()[2]), sizeof(float));
				}
				else if (Type == (int)AINBFile::ValueType::UserDefined)
				{
					Writer.WriteInteger(*reinterpret_cast<uint32_t*>(&Entry.Value), sizeof(uint32_t));
				}
			}
			for (AINBFile::OutputEntry Entry : OutputParameters[Type])
			{
				AddToStringTable(Entry.Name, StringTable);
				uint32_t Offset = GetOffsetInStringTable(Entry.Name, StringTable);
				if (Entry.SetPointerFlagsBitZero)
				{
					Writer.WriteInteger(Offset | (1 << 31), sizeof(uint32_t));
				}
				else
				{
					Writer.WriteInteger(Offset, sizeof(uint32_t));
				}
				if (Type == (int)AINBFile::ValueType::UserDefined)
				{
					AddToStringTable(Entry.Class, StringTable);
					Writer.WriteInteger(GetOffsetInStringTable(Entry.Class, StringTable), sizeof(uint32_t));
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < 12; i++)
		{
			Writer.WriteInteger(Current, sizeof(uint32_t));
		}
	}

	uint32_t MultiStart = Writer.GetPosition();
	if (!Multis.empty())
	{
		for (AINBFile::MultiEntry Entry : Multis)
		{
			Writer.WriteInteger(Entry.NodeIndex, sizeof(int16_t));
			Writer.WriteInteger(Entry.ParameterIndex, sizeof(int16_t));
			uint16_t Flags = 0x0;
			if (Entry.GlobalParametersIndex != 0xFFFF)
			{
				Writer.WriteInteger(Entry.GlobalParametersIndex, sizeof(uint16_t));
				Flags += 0x8000;
			}
			else //TODO: Exb
			{
				Writer.WriteInteger(0, sizeof(uint16_t));
			}
			if (!Entry.Flags.empty())
			{
				for (AINBFile::FlagsStruct Flag : Entry.Flags)
				{
					if (Flag == AINBFile::FlagsStruct::PulseThreadLocalStorage)
					{
						Flags += 0x80;
					}
					if (Flag == AINBFile::FlagsStruct::SetPointerFlagBitZero)
					{
						Flags += 0x100;
					}
				}
			}
			Writer.WriteInteger(Flags, sizeof(uint16_t));
		}
	}

	uint32_t ResidentStart = Writer.GetPosition();
	if (!Residents.empty())
	{
		Current = ResidentStart + Residents.size() * 4;
		int n = 0;
		for (int i = 0; i < Residents.size(); i++)
		{
			if (Residents[i].String != "")
			{
				n = 8;
			}
			else
			{
				n = 4;
			}
			Writer.WriteInteger(Current, sizeof(uint32_t));
			Current += n;
		}
		for (AINBFile::ResidentEntry Resident : Residents)
		{
			uint32_t Flags = 0;
			for (AINBFile::FlagsStruct Flag : Resident.Flags)
			{
				if (Flag == AINBFile::FlagsStruct::IsValidUpdate)
				{
					Flags  = Flags | 1;
				}
				if (Flag == AINBFile::FlagsStruct::UpdatePostCurrentCommandCalc)
				{
					Flags = Flags | (1 << 31);
				}
			}
			Writer.WriteInteger(Flags, sizeof(uint32_t));
			if (Resident.String != "")
			{
				AddToStringTable(Resident.String, StringTable);
				Writer.WriteInteger(GetOffsetInStringTable(Resident.String, StringTable), sizeof(uint32_t));
			}
		}
	}

	uint32_t PreconditionStart = Writer.GetPosition();
	if (!PreconditionNodes.empty())
	{
		for (int i = 0; i < PreconditionNodes.size(); i++)
		{
			std::cout << "WRITE PRECONDITION\n";
			Writer.WriteInteger(PreconditionNodes[i], sizeof(uint16_t));
			Writer.WriteInteger(0, sizeof(uint16_t)); //Purpose unknown
		}
	}

	uint32_t EXBStart = Writer.GetPosition();
	//TODO: EXB stuff

	uint32_t EmbedAINBStart = Writer.GetPosition();
	Writer.WriteInteger(this->EmbeddedAinbArray.size(), sizeof(uint32_t));
	for (AINBFile::EmbeddedAinb AINB : this->EmbeddedAinbArray)
	{
		AddToStringTable(AINB.FilePath, StringTable);
		AddToStringTable(AINB.FileCategory, StringTable);
		Writer.WriteInteger(GetOffsetInStringTable(AINB.FilePath, StringTable), sizeof(uint32_t));
		Writer.WriteInteger(GetOffsetInStringTable(AINB.FileCategory, StringTable), sizeof(uint32_t));
		Writer.WriteInteger(AINB.Count, sizeof(uint32_t));
	}

	//TODO: Entry Strings
	uint32_t EntryStringsStart = Writer.GetPosition();
	Writer.WriteInteger(0, sizeof(uint32_t)); //Entry Strings size

	uint32_t HashStart = Writer.GetPosition();
	Writer.WriteInteger(0, sizeof(uint64_t)); //Skipping two 32-bit hashes, removing them has no affect in game

	uint32_t ChildReplaceStart = Writer.GetPosition();
	Writer.WriteInteger(0, sizeof(uint16_t)); //Set at rutime
	if (!Replacements.empty())
	{
		Writer.WriteInteger(Replacements.size(), sizeof(uint16_t));
		uint32_t AttachCount = 0;
		uint32_t NodeCount = 0;
		int16_t OverrideNode = this->Nodes.size();
		int16_t OverrideAttach = this->AttachmentParameters.size();

		for (WriterReplacement Replacement : Replacements)
		{
			if (Replacement.Type == 2)
			{
				AttachCount += 1;
				OverrideAttach -= 1;
			}
			if (Replacement.Type == 0 || Replacement.Type == 1)
			{
				NodeCount += 1;
				if (Replacement.Type == 0)
				{
					OverrideNode -= 1;
				}
				if (Replacement.Type == 1)
				{
					OverrideNode -= 2;
				}
			}
		}
		if (NodeCount > 0)
		{
			Writer.WriteInteger(OverrideNode, sizeof(int16_t));
		}
		else
		{
			Writer.WriteInteger(-1, sizeof(int16_t));
		}
		if (AttachCount > 0)
		{
			Writer.WriteInteger(OverrideAttach, sizeof(int16_t));
		}
		else
		{
			Writer.WriteInteger(-1, sizeof(int16_t));
		}
		for (WriterReplacement Replacement : Replacements)
		{
			Writer.WriteInteger(Replacement.Type, sizeof(uint8_t));
			Writer.WriteInteger(0, sizeof(uint8_t));
			Writer.WriteInteger(Replacement.NodeIndex, sizeof(uint16_t));
			Writer.WriteInteger(Replacement.Iteration, sizeof(uint16_t));
			if (Replacement.Type == 1)
			{
				Writer.WriteInteger(Replacement.ReplacementNodeIndex, sizeof(uint16_t));
			}
		}
	}
	else
	{
		Writer.WriteInteger(0, sizeof(uint16_t));
		Writer.WriteInteger(-1, sizeof(int16_t));
		Writer.WriteInteger(-1, sizeof(int16_t));
	}

	uint32_t x6cStart = Writer.GetPosition();
	Writer.WriteInteger(0, sizeof(uint32_t));
	uint32_t ResolveStart = Writer.GetPosition();
	Writer.WriteInteger(0, sizeof(uint32_t));
	uint32_t StringsStart = Writer.GetPosition();

	for (std::string String : StringTable)
	{
		Writer.WriteBytes(String.c_str());
		Writer.WriteByte(0x00);
	}

	Writer.Seek(24, BinaryVectorWriter::Position::Begin);
	Writer.WriteInteger(Attachments.size(), sizeof(uint32_t));
	Writer.Seek(36, BinaryVectorWriter::Position::Begin);
	Writer.WriteInteger(StringsStart, sizeof(uint32_t));
	Writer.WriteInteger(ResolveStart, sizeof(uint32_t));
	Writer.WriteInteger(ImmediateStart, sizeof(uint32_t));
	Writer.WriteInteger(ResidentStart, sizeof(uint32_t));
	Writer.WriteInteger(IOStart, sizeof(uint32_t));
	Writer.WriteInteger(MultiStart, sizeof(uint32_t));
	Writer.WriteInteger(AttachmentStart, sizeof(uint32_t));
	Writer.WriteInteger(AttachmentIndexStart, sizeof(uint32_t));
	Writer.WriteInteger(0, sizeof(uint32_t)); //TODO: EXB Offset
	Writer.WriteInteger(ChildReplaceStart, sizeof(uint32_t));
	Writer.WriteInteger(PreconditionStart, sizeof(uint32_t));
	Writer.WriteInteger(ResidentStart, sizeof(uint32_t)); //Always the same
	Writer.Seek(8, BinaryVectorWriter::Position::Current);
	Writer.WriteInteger(EmbedAINBStart, sizeof(uint32_t));
	Writer.Seek(8, BinaryVectorWriter::Position::Current);
	Writer.WriteInteger(EntryStringsStart, sizeof(uint32_t));
	Writer.WriteInteger(x6cStart, sizeof(uint32_t));
	Writer.WriteInteger(HashStart, sizeof(uint32_t));

	return Writer.GetData();
}

//I hate this file format, 2200 lines of code, just for reading and writing a file with 1000 bytes...

void AINBFile::Write(std::string Path)
{
    std::ofstream File(Path, std::ios::binary);
    std::vector<unsigned char> Binary = this->ToBinary();
    std::copy(Binary.cbegin(), Binary.cend(),
        std::ostream_iterator<unsigned char>(File));
    File.close();
}