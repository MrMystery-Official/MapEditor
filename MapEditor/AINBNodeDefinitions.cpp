#include "AINBNodeDefinitions.h"
#include "Config.h"

std::vector<AINBNodeDefinitions::NodeDefinition> AINBNodeDefinitions::NodeDefinitions;

std::string ReadString(BinaryVectorReader& Reader)
{
	int Length = Reader.ReadUInt32();
	std::string Result;
	for (int i = 0; i < Length; i++)
	{
		Result += Reader.ReadInt8();
	}
	return Result;
}

void AINBNodeDefinitions::Initialize()
{
	std::ifstream File(Config::GetWorkingDirFile("Definitions.eainbdef"), std::ios::binary);

	if (!File.eof() && !File.fail())
	{
		File.seekg(0, std::ios_base::end);
		std::streampos FileSize = File.tellg();

		std::vector<unsigned char> Bytes(FileSize);

		File.seekg(0, std::ios_base::beg);
		File.read(reinterpret_cast<char*>(Bytes.data()), FileSize);
		File.close();

		
		BinaryVectorReader Reader(Bytes);
		Reader.Seek(9, BinaryVectorReader::Position::Begin); //Skip magic + version

		AINBNodeDefinitions::NodeDefinitions.resize(Reader.ReadUInt32());

		std::vector<uint32_t> Offsets(AINBNodeDefinitions::NodeDefinitions.size());
		for (int i = 0; i < AINBNodeDefinitions::NodeDefinitions.size(); i++)
		{
			Offsets[i] = Reader.ReadUInt32();
		}

		for (int i = 0; i < AINBNodeDefinitions::NodeDefinitions.size(); i++)
		{
			Reader.Seek(Offsets[i], BinaryVectorReader::Position::Begin);
			AINBNodeDefinitions::NodeDefinition Definition;
			Definition.Name = ReadString(Reader);
			Definition.NameHash = Reader.ReadUInt32();
			Definition.Type = Reader.ReadUInt16();
			Definition.Category = (AINBNodeDefinitions::NodeDefinitionCategory)Reader.ReadUInt8();
			for (int Type = 0; Type < AINBFile::ValueTypeCount; Type++)
			{
				Definition.InputParameters[Type].resize(Reader.ReadUInt8());
				Definition.OutputParameters[Type].resize(Reader.ReadUInt8());
				Definition.ImmediateParameters[Type].resize(Reader.ReadUInt8());
			}
			for (int Type = 0; Type < AINBFile::ValueTypeCount; Type++)
			{
				for (int i = 0; i < Definition.InputParameters[Type].size(); i++)
				{
					AINBNodeDefinitions::NodeDefinitionInputParameter Param;
					Param.Name = ReadString(Reader);
					Param.Class = ReadString(Reader);
					Param.ValueType = (AINBFile::ValueType)Reader.ReadUInt8();

					switch (Param.ValueType)
					{
					case AINBFile::ValueType::Bool:
						Param.Value = (bool)Reader.ReadUInt8();
						break;
					case AINBFile::ValueType::Int:
						Param.Value = Reader.ReadUInt32();
						break;
					case AINBFile::ValueType::Float:
						Param.Value = Reader.ReadFloat();
						break;
					case AINBFile::ValueType::String:
						Param.Value = ReadString(Reader);
						break;
					case AINBFile::ValueType::Vec3f:
						Param.Value = Vector3F(Reader.ReadFloat(), Reader.ReadFloat(), Reader.ReadFloat());
						break;
					}
					Definition.InputParameters[Type][i] = Param;
				}

				for (int i = 0; i < Definition.OutputParameters[Type].size(); i++)
				{
					AINBNodeDefinitions::NodeDefinitionOutputParameter Param;
					Param.Name = ReadString(Reader);
					Param.Class = ReadString(Reader);
					Param.SetPointerFlagsBitZero = (bool)Reader.ReadUInt8();
					Definition.OutputParameters[Type][i] = Param;
				}

				for (int i = 0; i < Definition.ImmediateParameters[Type].size(); i++)
				{
					AINBNodeDefinitions::NodeDefinitionImmediateParameter Param;
					Param.Name = ReadString(Reader);
					Param.Class = ReadString(Reader);
					Param.ValueType = (AINBFile::ValueType)Reader.ReadUInt8();
					Definition.ImmediateParameters[Type][i] = Param;
				}
			}
			AINBNodeDefinitions::NodeDefinitions[i] = Definition;
		}
	}
}

AINBNodeDefinitions::NodeDefinition* AINBNodeDefinitions::GetNodeDefinition(std::string Name)
{
	for (AINBNodeDefinitions::NodeDefinition& Def : AINBNodeDefinitions::NodeDefinitions)
	{
		if (Def.Name == Name)
		{
			return &Def;
		}
	}
	return nullptr;
}

void AINBNodeDefinitions::Generate()
{
	std::vector<AINBNodeDefinitions::NodeDefinition> Definitions;

	using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
	for (const auto& DirEntry : recursive_directory_iterator(Config::GetRomFSFile("Logic", false)))
	{
		std::cout << DirEntry.path().string() << std::endl;
		if (DirEntry.is_regular_file())
		{
			AINBFile File(DirEntry.path().string());
			bool NodeFound = false;
			for (AINBFile::Node Node : File.Nodes)
			{
				for (AINBNodeDefinitions::NodeDefinition& Definition : Definitions)
				{
					if (Definition.Name == (Node.Name.length() > 0 ? Node.Name : AINBFile::NodeTypeToString((AINBFile::NodeTypes)Node.Type)))
					{
						NodeFound = true;
						for (int i = 0; i < AINBFile::ValueTypeCount; i++)
						{
							for (AINBFile::OutputEntry Param : Node.OutputParameters[i])
							{
								bool Found = false;
								for (AINBNodeDefinitions::NodeDefinitionOutputParameter DefParam : Definition.OutputParameters[i])
								{
									if (DefParam.Name == Param.Name)
									{
										Found = true;
										break;
									}
								}
								if (Found) continue;
								Definition.OutputParameters[i].push_back({ Param.Name, Param.Class, Param.SetPointerFlagsBitZero });
							}
							for (AINBFile::InputEntry Param : Node.InputParameters[i])
							{
								bool Found = false;
								for (AINBNodeDefinitions::NodeDefinitionInputParameter DefParam : Definition.InputParameters[i])
								{
									if (DefParam.Name == Param.Name)
									{
										Found = true;
										break;
									}
								}
								if (Found) continue;
								Definition.InputParameters[i].push_back({ Param.Name, Param.Class, Param.Value, (AINBFile::ValueType)Param.ValueType });
							}
							for (AINBFile::ImmediateParameter Param : Node.ImmediateParameters[i])
							{
								bool Found = false;
								for (AINBNodeDefinitions::NodeDefinitionImmediateParameter DefParam : Definition.ImmediateParameters[i])
								{
									if (DefParam.Name == Param.Name)
									{
										Found = true;
										break;
									}
								}
								if (Found) continue;
								Definition.ImmediateParameters[i].push_back({ Param.Name, Param.Class, (AINBFile::ValueType)Param.ValueType });
							}
						}
						break;
					}
				}
				if (NodeFound) continue;
				AINBNodeDefinitions::NodeDefinition Definition;
				Definition.Category = File.Header.FileCategory == "AI" ? AINBNodeDefinitions::NodeDefinitionCategory::AI : (File.Header.FileCategory == "Logic" ? AINBNodeDefinitions::NodeDefinitionCategory::Logic : AINBNodeDefinitions::NodeDefinitionCategory::Sequence);
				Definition.Name = Node.Name;
				Definition.NameHash = Node.NameHash;
				Definition.Type = Node.Type;
				if (Definition.Name.length() == 0)
				{
					Definition.Name = AINBFile::NodeTypeToString((AINBFile::NodeTypes)Definition.Type);
					std::cout << Definition.Type << std::endl;
				}
				for (int i = 0; i < AINBFile::ValueTypeCount; i++)
				{
					for (AINBFile::OutputEntry Param : Node.OutputParameters[i])
					{
						Definition.OutputParameters[i].push_back({ Param.Name, Param.Class, Param.SetPointerFlagsBitZero });
					}
					for (AINBFile::InputEntry Param : Node.InputParameters[i])
					{
						Definition.InputParameters[i].push_back({ Param.Name, Param.Class, Param.Value, (AINBFile::ValueType)Param.ValueType });
					}
					for (AINBFile::ImmediateParameter Param : Node.ImmediateParameters[i])
					{
						Definition.ImmediateParameters[i].push_back({ Param.Name, Param.Class, (AINBFile::ValueType)Param.ValueType });
					}
				}
				Definitions.push_back(Definition);
			}
		}
	}

	BinaryVectorWriter Writer;
	Writer.WriteBytes("EAINBDEF"); //Magic
	Writer.WriteByte(0x01); //Version

	Writer.WriteInteger(Definitions.size(), sizeof(uint32_t)); //Definition count

	Writer.Seek(Definitions.size() * 4, BinaryVectorWriter::Position::Current); //Skip offsets until known

	std::vector<uint32_t> Offsets;

	for (AINBNodeDefinitions::NodeDefinition Def : Definitions)
	{
		Offsets.push_back(Writer.GetPosition());
		Writer.WriteInteger(Def.Name.length(), sizeof(uint32_t)); //Name length
		Writer.WriteBytes(Def.Name.c_str());
		Writer.WriteInteger(Def.NameHash, sizeof(uint32_t));
		Writer.WriteInteger(Def.Type, sizeof(uint16_t));
		Writer.WriteByte((uint8_t)Def.Category);
		for (int Type = 0; Type < AINBFile::ValueTypeCount; Type++)
		{
			Writer.WriteInteger(Def.InputParameters[Type].size(), sizeof(uint8_t));
			Writer.WriteInteger(Def.OutputParameters[Type].size(), sizeof(uint8_t));
			Writer.WriteInteger(Def.ImmediateParameters[Type].size(), sizeof(uint8_t));
		}
		for (int Type = 0; Type < AINBFile::ValueTypeCount; Type++)
		{
			for (AINBNodeDefinitions::NodeDefinitionInputParameter Param : Def.InputParameters[Type])
			{
				Writer.WriteInteger(Param.Name.size(), sizeof(uint32_t));
				Writer.WriteBytes(Param.Name.c_str());
				Writer.WriteInteger(Param.Class.size(), sizeof(uint32_t));
				Writer.WriteBytes(Param.Class.c_str());
				Writer.WriteInteger((uint8_t)Param.ValueType, sizeof(uint8_t));
				switch (Param.ValueType)
				{
				case AINBFile::ValueType::Bool:
					Writer.WriteInteger(*reinterpret_cast<bool*>(&Param.Value), sizeof(bool));
					break;
				case AINBFile::ValueType::Int:
					Writer.WriteInteger(*reinterpret_cast<int*>(&Param.Value), sizeof(int32_t));
					break;
				case AINBFile::ValueType::Float:
					Writer.WriteRawUnsafeFixed(reinterpret_cast<char*>(&Param.Value), sizeof(float));
					break;
				case AINBFile::ValueType::String:
					Writer.WriteInteger(reinterpret_cast<std::string*>(&Param.Value)->length(), sizeof(uint32_t));
					Writer.WriteBytes(reinterpret_cast<std::string*>(&Param.Value)->c_str());
					break;
				case AINBFile::ValueType::Vec3f:
					Vector3F Vec3f = *reinterpret_cast<Vector3F*>(&Param.Value);
					Writer.WriteRawUnsafeFixed(reinterpret_cast<char*>(&Vec3f.GetRawData()[0]), sizeof(float));
					Writer.WriteRawUnsafeFixed(reinterpret_cast<char*>(&Vec3f.GetRawData()[1]), sizeof(float));
					Writer.WriteRawUnsafeFixed(reinterpret_cast<char*>(&Vec3f.GetRawData()[2]), sizeof(float));
					break;
				}
			}
			for (AINBNodeDefinitions::NodeDefinitionOutputParameter Param : Def.OutputParameters[Type])
			{
				Writer.WriteInteger(Param.Name.size(), sizeof(uint32_t));
				Writer.WriteBytes(Param.Name.c_str());
				Writer.WriteInteger(Param.Class.size(), sizeof(uint32_t));
				Writer.WriteBytes(Param.Class.c_str());
				Writer.WriteInteger((uint8_t)Param.SetPointerFlagsBitZero, sizeof(uint8_t));
			}
			for (AINBNodeDefinitions::NodeDefinitionImmediateParameter Param : Def.ImmediateParameters[Type])
			{
				Writer.WriteInteger(Param.Name.size(), sizeof(uint32_t));
				Writer.WriteBytes(Param.Name.c_str());
				Writer.WriteInteger(Param.Class.size(), sizeof(uint32_t));
				Writer.WriteBytes(Param.Class.c_str());
				Writer.WriteInteger((uint8_t)Param.ValueType, sizeof(uint8_t));
			}
		}
	}

	Writer.Seek(13, BinaryVectorWriter::Position::Begin);
	for (uint32_t Offset : Offsets)
	{
		Writer.WriteInteger(Offset, sizeof(uint32_t));
	}
	
	std::ofstream FileOut(Config::GetWorkingDirFile("Definitions.eainbdef"), std::ios::binary);
	std::vector<unsigned char> Binary = Writer.GetData();
	std::copy(Binary.cbegin(), Binary.cend(),
		std::ostream_iterator<unsigned char>(FileOut));
	FileOut.close();
}