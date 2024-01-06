#include "Byml.h"


/* --- Node --- */
BymlFile::Node::Node(BymlFile::Type Type, std::string Key) : m_Type(Type), m_Key(Key)
{
}

BymlFile::Type& BymlFile::Node::GetType()
{
    return this->m_Type;
}

std::string& BymlFile::Node::GetKey()
{
    return this->m_Key;
}

template<> bool BymlFile::Node::GetValue() //For booleans
{
    return this->m_Value[0];
}

template<> std::string BymlFile::Node::GetValue() //For strings
{
    std::vector<unsigned char> Input = this->m_Value;

    if (this->m_Type != BymlFile::Type::StringIndex)
    {
        std::string Number = "";
        switch (this->m_Type)
        {
        case BymlFile::Type::UInt32:
            Number = std::to_string(this->GetValue<uint32_t>());
            break;
        case BymlFile::Type::UInt64:
            Number = std::to_string(this->GetValue<uint64_t>());
            break;
        case BymlFile::Type::Int32:
            Number = std::to_string(this->GetValue<int32_t>());
            break;
        case BymlFile::Type::Int64:
            Number = std::to_string(this->GetValue<int64_t>());
            break;
        case BymlFile::Type::Float:
            Number = std::to_string(this->GetValue<float>());
            break;
        case BymlFile::Type::Bool:
            return this->GetValue<bool>() ? "true" : "false";
        }

        Input.resize(Number.length());
        memcpy(Input.data(), Number.data(), Number.length());
    }

    std::string Result;
    for (char Character : Input)
    {
        Result += Character;
    }
    return Result;
}

template<class T> T BymlFile::Node::GetValue() //For all numbers
{
    T Value;
    memcpy(&Value, this->m_Value.data(), sizeof(Value));
    return Value;
}

template<> void BymlFile::Node::SetValue(std::string Value) //Value os a string
{
    this->m_Value.resize(Value.length());
    memcpy(this->m_Value.data(), Value.data(), Value.length());
}

template<class T> void BymlFile::Node::SetValue(T Value) //Value is a default C++ type
{
    this->m_Value.resize(sizeof(Value));
    memcpy(this->m_Value.data(), &Value, sizeof(Value));
}

std::vector<BymlFile::Node>& BymlFile::Node::GetChildren()
{
    return this->m_Children;
}

void BymlFile::Node::AddChild(BymlFile::Node& Node)
{
    this->m_Children.push_back(Node);
}

bool BymlFile::Node::HasChild(std::string Key)
{
    for (BymlFile::Node& Node : this->m_Children)
    {
        if (Node.m_Key == Key)
        {
            return true;
        }
    }
    return false;
}

BymlFile::Node* BymlFile::Node::GetChild(std::string Key)
{
    for (BymlFile::Node& Node : this->m_Children)
    {
        if (Node.m_Key == Key)
        {
            return &Node;
        }
    }

    return nullptr;
}

BymlFile::Node* BymlFile::Node::GetChild(int Index)
{
    return &this->m_Children[Index];
}
/* --- Node --- */

bool BymlFile::HasChild(std::string Key)
{
    for (BymlFile::Node& Node : this->m_Nodes)
    {
        if (Node.GetKey() == Key)
        {
            return true;
        }
    }
    return false;
}

void BymlFile::ParseTable(BinaryVectorReader& Reader, std::vector<std::string>* Dest, int TableOffset)
{
    Reader.Seek(TableOffset, BinaryVectorReader::Position::Begin);
    if (Reader.ReadUInt8() != (uint8_t)BymlFile::Type::KeyTable)
    {
        std::cerr << "Could not parse key table, because the node type was wrong. (expected 0xC2)\n";
        return;
    }
    uint32_t TableSize = Reader.ReadUInt24();
    Dest->resize(TableSize);
    for (uint32_t i = 0; i < TableSize; i++)
    {
        Reader.Seek(TableOffset + 4 + 4 * i, BinaryVectorReader::Position::Begin);
        Reader.Seek(TableOffset + Reader.ReadUInt32(), BinaryVectorReader::Position::Begin);
        char CurrentCharacter = Reader.ReadInt8();
        Dest->at(i) += CurrentCharacter;
        while (CurrentCharacter != 0x00)
        {
            CurrentCharacter = Reader.ReadInt8();
            Dest->at(i) += CurrentCharacter;
        }
        Dest->at(i).pop_back();
    }
}

int BymlFile::AlignUp(int Value, int Size)
{
    return Value + (Size - Value % Size) % Size;
}

void BymlFile::ParseNode(BinaryVectorReader& Reader, int Offset, BymlFile::Type Type, std::string Key, BymlFile::Node* Parent)
{
    if (Type == BymlFile::Type::Null)
    {
        std::cerr << "Got BYML Node with Null type!\n";
        return;
    }

    BymlFile::Node Node(Type, Key);
    Reader.Seek(Offset, BinaryVectorReader::Position::Begin);

    switch (Type)
    {
    case BymlFile::Type::Double:
    {
        Node.SetValue<double>(Reader.ReadDouble());
        break;
    }
    case BymlFile::Type::Float:
    {
        Node.SetValue<float>(Reader.ReadFloat());
        break;
    }
    case BymlFile::Type::UInt32:
    {
        Node.SetValue<uint32_t>(Reader.ReadUInt32());
        break;
    }
    case BymlFile::Type::Int32:
    {
        Node.SetValue<int32_t>(Reader.ReadInt32());
        break;
    }
    case BymlFile::Type::UInt64:
    {
        Reader.Seek(Reader.ReadInt32(), BinaryVectorReader::Position::Begin);
        Node.SetValue<uint64_t>(Reader.ReadUInt64());
        break;
    }
    case BymlFile::Type::Int64:
    {
        Reader.Seek(Reader.ReadInt32(), BinaryVectorReader::Position::Begin);
        Node.SetValue<int64_t>(Reader.ReadInt64());
        break;
    }
    case BymlFile::Type::Bool:
    {
        Node.SetValue<bool>(Reader.ReadUInt32());
        break;
    }
    case BymlFile::Type::StringIndex:
    {
        Node.SetValue<std::string>(this->m_StringTable[Reader.ReadUInt32()]);
        break;
    }
    case BymlFile::Type::Array:
    {
        uint32_t LocalOffset = Reader.ReadUInt32();
        Reader.Seek(LocalOffset + 1, BinaryVectorReader::Position::Begin);
        uint32_t Size = Reader.ReadUInt24();
        uint32_t ValueArrayOffset = LocalOffset + 4 + this->AlignUp(Size, 4);
        for (uint32_t i = 0; i < Size; i++)
        {
            Reader.Seek(LocalOffset + 4 + i, BinaryVectorReader::Position::Begin);
            this->ParseNode(Reader, ValueArrayOffset + 4 * i, static_cast<BymlFile::Type>(Reader.ReadUInt8()), std::to_string(i), &Node);
        }
        break;
    }
    case BymlFile::Type::Dictionary:
    {
        uint32_t LocalOffset = Reader.ReadUInt32();
        Reader.Seek(LocalOffset + 1, BinaryVectorReader::Position::Begin);
        uint32_t Size = Reader.ReadUInt24();
        for (uint32_t i = 0; i < Size; i++)
        {
            uint32_t EntryOffset = LocalOffset + 4 + 8 * i;
            Reader.Seek(EntryOffset, BinaryVectorReader::Position::Begin);
            uint32_t StringOffset = Reader.ReadUInt24();
            Reader.Seek(EntryOffset + 3, BinaryVectorReader::Position::Begin);
            this->ParseNode(Reader, EntryOffset + 4, static_cast<BymlFile::Type>(Reader.ReadUInt8()), this->m_HashKeyTable[StringOffset], &Node);
        }
        break;
    }
    default:
        std::cerr << "Unsupported node type " << (int)Type << "!\n";
    }

    Parent->AddChild(Node);
}

std::vector<BymlFile::Node>& BymlFile::GetNodes()
{
    return this->m_Nodes;
}

BymlFile::Type& BymlFile::GetType()
{
    return this->m_Type;
}

BymlFile::Node* BymlFile::GetNode(std::string Key)
{
    for (BymlFile::Node& Node : this->m_Nodes)
    {
        if (Node.GetKey() == Key)
        {
            return &Node;
        }
    }

    return nullptr;
}

void BymlFile::AddHashKeyTableEntry(std::string Key)
{
    this->m_HashKeyTable.push_back(Key);
}

void BymlFile::AddStringTableEntry(std::string String)
{
    this->m_StringTable.push_back(String);
}

bool BymlFile::IsNumber(const std::string& str)
{
    if (str.empty())
    {
        return false;
    }

    bool hasDigit = false;
    bool hasDecimal = false;
    bool hasSign = false;

    for (char c : str)
    {
        if (isdigit(c))
        {
            hasDigit = true;
        }
        else if (c == '.' && !hasDecimal)
        {
            hasDecimal = true;
        }
        else if ((c == '-' || c == '+') && !hasSign && !hasDigit)
        {
            hasSign = true;
        }
        else
        {
            return false;
        }
    }

    return hasDigit;
}

void BymlFile::GenerateHashKeyTable(BymlFile::Node* Node)
{
    std::string str = Node->GetKey();
    if (!str.empty())
    {
        if (std::find(this->m_HashKeyTable.begin(), this->m_HashKeyTable.end(), str) == this->m_HashKeyTable.end() && !this->IsNumber(str))
        {
            this->m_HashKeyTable.push_back(str);
        }
    }

    for (BymlFile::Node& Nodes : Node->GetChildren())
    {
        this->GenerateHashKeyTable(&Nodes);
    }
}

void BymlFile::GenerateStringTable(BymlFile::Node* Node)
{
    if (Node->GetType() == BymlFile::Type::StringIndex)
    {
        std::string str = Node->GetValue<std::string>();
        if (str.empty())
        {
            this->m_StringTable.push_back(str);
        }
        else if (std::find(this->m_StringTable.begin(), this->m_StringTable.end(), str) == this->m_StringTable.end())
        {
            this->m_StringTable.push_back(str);
        }
    }

    for (BymlFile::Node& Nodes : Node->GetChildren())
    {
        this->GenerateStringTable(&Nodes);
    }
}

uint32_t BymlFile::GetStringTableIndex(std::string Value)
{
    for (int i = 0; i < this->m_StringTable.size(); i++)
    {
        if (this->m_StringTable[i] == Value)
        {
            return i;
        }
    }
    return 0;
}

uint32_t BymlFile::GetHashKeyTableIndex(std::string Value)
{
    for (int i = 0; i < this->m_HashKeyTable.size(); i++)
    {
        if (this->m_HashKeyTable[i] == Value)
        {
            return i;
        }
    }
    return 0;
}

std::string BymlFile::GenerateNodeHash(BymlFile::Node* Node)
{
    std::string Result = (char)Node->GetType() + Node->GetKey() + Node->GetValue<std::string>();
    for (BymlFile::Node& Child : Node->GetChildren())
    {
        Result += GenerateNodeHash(&Child);
    }
    return Result;
}

void BymlFile::WriteNode(BinaryVectorWriter& Writer, uint32_t DataOffset, uint32_t Offset, BymlFile::Node* Node)
{
    if (Node->GetType() == BymlFile::Type::Null)
    {
        return;
    }

    Writer.Seek(Offset, BinaryVectorWriter::Position::Begin);

    switch (Node->GetType())
    {
    case BymlFile::Type::Float:
    {
        float Value = Node->GetValue<float>();
        Writer.WriteRawUnsafeFixed(reinterpret_cast<const char*>(&Value), sizeof(float));
        break;
    }
    case BymlFile::Type::UInt32:
    {
        Writer.WriteInteger(Node->GetValue<uint32_t>(), sizeof(uint32_t));
        break;
    }
    case BymlFile::Type::Int32:
    {
        Writer.WriteInteger(Node->GetValue<int32_t>(), sizeof(int32_t));
        break;
    }
    case BymlFile::Type::Bool:
    {
        Writer.WriteInteger(Node->GetValue<bool>(), sizeof(uint32_t));
        break;
    }
    case BymlFile::Type::StringIndex:
    {
        Writer.WriteInteger(GetStringTableIndex(Node->GetValue<std::string>()), sizeof(uint32_t));
        break;
    }
    case BymlFile::Type::UInt64:
    {
        Writer.WriteInteger(DataOffset + this->m_WriterReservedDataOffset, sizeof(uint32_t));
        Writer.Seek(DataOffset + this->m_WriterReservedDataOffset, BinaryVectorWriter::Position::Begin);
        Writer.WriteInteger(Node->GetValue<uint64_t>(), sizeof(uint64_t));
        this->m_WriterReservedDataOffset += 8;
        break;
    }
    case BymlFile::Type::Int64:
    {
        Writer.WriteInteger(DataOffset + this->m_WriterReservedDataOffset, sizeof(uint32_t));
        Writer.Seek(DataOffset + this->m_WriterReservedDataOffset, BinaryVectorWriter::Position::Begin);
        Writer.WriteInteger(Node->GetValue<int64_t>(), sizeof(int64_t));
        this->m_WriterReservedDataOffset += 8;
        break;
    }
    case BymlFile::Type::Double:
    {
        Writer.WriteInteger(DataOffset + this->m_WriterReservedDataOffset, sizeof(uint32_t));
        Writer.Seek(DataOffset + this->m_WriterReservedDataOffset, BinaryVectorWriter::Position::Begin);
        Writer.WriteInteger(Node->GetValue<double>(), sizeof(double));
        this->m_WriterReservedDataOffset += 8;
        break;
    }
    case BymlFile::Type::Array:
    {
        Writer.WriteInteger(DataOffset + this->m_WriterReservedDataOffset, sizeof(uint32_t));
        Writer.Seek(DataOffset + this->m_WriterReservedDataOffset, BinaryVectorWriter::Position::Begin);
        Writer.WriteByte(0xC0);
        Writer.WriteInteger(Node->GetChildren().size(), 3); //3 = uint24
        for (BymlFile::Node& Child : Node->GetChildren())
        {
            Writer.WriteByte((uint8_t)Child.GetType());
        }
        while (Writer.GetPosition() % 4 != 0) //Aligning up after array content by 4 bytes
        {
            Writer.WriteByte(0x00);
        }
        this->m_WriterReservedDataOffset = Writer.GetPosition() - DataOffset + 4 * Node->GetChildren().size();
        uint32_t EntryOffset = Writer.GetPosition();
        for (int i = 0; i < Node->GetChildren().size(); i++)
        {
            this->WriteNode(Writer, DataOffset, EntryOffset + 4 * i, Node->GetChild(i));
        }
        break;
    }
    case BymlFile::Type::Dictionary:
    {
        uint32_t CloneOffset = 0;
        std::string Hash;
        if (this->NodeCaching)
        {
            Hash = GenerateNodeHash(Node);
            for (auto const& [Key, Value] : this->m_CachedNodes) {
                if (Key == Hash) {
                    CloneOffset = Value;
                    break;
                }
            }
        }

        if (CloneOffset == 0)
        {
            uint32_t LocalOffset = DataOffset + this->m_WriterReservedDataOffset;
            Writer.WriteInteger(LocalOffset, sizeof(uint32_t));
            this->m_WriterReservedDataOffset += 4 + 8 * Node->GetChildren().size();
            Writer.Seek(LocalOffset, BinaryVectorWriter::Position::Begin);
            Writer.WriteByte(0xC1);
            Writer.WriteInteger(Node->GetChildren().size(), 3); // 3 = uint24
            for (int i = 0; i < Node->GetChildren().size(); i++)
            {
                uint32_t EntryOffset = LocalOffset + 4 + 8 * i;
                Writer.Seek(EntryOffset, BinaryVectorWriter::Position::Begin);
                Writer.WriteInteger(GetHashKeyTableIndex(Node->GetChild(i)->GetKey()), 3);
                Writer.WriteByte((char)Node->GetChild(i)->GetType());
                this->WriteNode(Writer, DataOffset, EntryOffset + 4, Node->GetChild(i));
                while (Writer.GetPosition() % 4 != 0) //Aligning up after dict content by 4 bytes
                {
                    Writer.WriteByte(0x00);
                }
            }
            if (this->NodeCaching)
                this->m_CachedNodes.insert({ Hash, LocalOffset });
        }
        else
        {
            Writer.WriteInteger(CloneOffset, sizeof(uint32_t));
        }
        break;
    }
    default:
        std::cerr << "Unsupported node type: " << (int)Node->GetType() << "!\n";
    }

    if (Writer.GetPosition() > this->m_WriterLastOffset)
    {
        this->m_WriterLastOffset = Writer.GetPosition();
    }
}

std::vector<unsigned char> BymlFile::ToBinary(BymlFile::TableGeneration TableGeneration)
{
    std::chrono::milliseconds StartMS = duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    );

    this->m_WriterLastOffset = 0;
    this->m_WriterReservedDataOffset = 0;
    this->m_CachedNodes.clear();

    BinaryVectorWriter Writer;

    Writer.WriteBytes("YB"); //Magic
    Writer.WriteInteger(0x07, sizeof(uint16_t)); //Version
    Writer.WriteInteger(0x10, sizeof(uint32_t)); //Hash Key Table Offset

    Writer.Seek(8, BinaryVectorWriter::Position::Current);

    if (TableGeneration == BymlFile::TableGeneration::Auto)
    {
        this->m_HashKeyTable.clear();
        this->m_HashKeyTable.resize(0);

        this->m_StringTable.clear();
        this->m_StringTable.resize(0);

        for (BymlFile::Node& Node : this->m_Nodes)
        {
            this->GenerateHashKeyTable(&Node);
            this->GenerateStringTable(&Node);
        }
    }
    this->m_HashKeyTable.shrink_to_fit();
    this->m_StringTable.shrink_to_fit();
    std::sort(this->m_HashKeyTable.begin(), this->m_HashKeyTable.end(), [](std::string A, std::string B) { return A < B; });
    std::sort(this->m_StringTable.begin(), this->m_StringTable.end(), [](std::string A, std::string B) { return A < B; });

    Writer.WriteByte(0xC2); //Key Table Node
    Writer.WriteInteger(this->m_HashKeyTable.size(), 3); //3 = uint24
    Writer.Seek(this->m_HashKeyTable.size() * 4 + 4, BinaryVectorWriter::Position::Current);

    std::vector<uint32_t> HashKeyOffsets;

    for (std::string& HashKey : this->m_HashKeyTable)
    {
        HashKeyOffsets.push_back(Writer.GetPosition() - 16);
        Writer.WriteBytes(HashKey.c_str());
        Writer.WriteByte(0xC00);
    }
    HashKeyOffsets.push_back(Writer.GetPosition() - 16);
    while (Writer.GetPosition() % 4 != 0) //Align
    {
        Writer.WriteByte(0x00);
    }

    
    Writer.WriteByte(0xC2);
    Writer.WriteInteger(this->m_StringTable.size(), 3); //3 = uint24
    uint32_t StringTableJumpback = Writer.GetPosition();
    Writer.Seek(this->m_StringTable.size() * 4 + 4, BinaryVectorWriter::Position::Current);
    std::vector<uint32_t> StringOffsets;
    for (std::string String : this->m_StringTable)
    {
        StringOffsets.push_back(Writer.GetPosition() - StringTableJumpback + 4); //-16 because it is a relative offset
        Writer.WriteBytes(String.c_str());
        Writer.WriteByte(0x00);
    }
    StringOffsets.push_back(Writer.GetPosition() - StringTableJumpback + 4); //For the last string in the table
    while (Writer.GetPosition() % 4 != 0) //Align
    {
        Writer.WriteByte(0x00);
    }

    uint32_t DataOffset = Writer.GetPosition();

    //Write nodes
    BymlFile::Node RootNode(this->m_Type, "root");
    for (BymlFile::Node& Node : this->m_Nodes)
    {
        RootNode.AddChild(Node);
    }
    this->WriteNode(Writer, DataOffset, 12, &RootNode);

    Writer.Seek(0x08, BinaryVectorWriter::Position::Begin); //Jump to the string table offset
    Writer.WriteInteger(StringTableJumpback - 4, sizeof(uint32_t));

    Writer.Seek(0x14, BinaryVectorWriter::Position::Begin);
    for (uint32_t HashKeyOffset : HashKeyOffsets)
    {
        Writer.WriteInteger(HashKeyOffset, sizeof(uint32_t));
    }

    Writer.Seek(StringTableJumpback, BinaryVectorWriter::Position::Begin);
    for (uint32_t StringOffset : StringOffsets)
    {
        Writer.WriteInteger(StringOffset, sizeof(uint32_t));
    }

    //std::vector<unsigned char> Data = Writer.GetData();
    //Data.resize(Data.size() - 1);

    std::cout << "Finished, time needed: " << (duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ) - StartMS) << "!" << std::endl;

    return Writer.GetData();
}

void BymlFile::WriteToFile(std::string Path, BymlFile::TableGeneration TableGeneration)
{
    std::ofstream File(Path, std::ios::binary);
    std::vector<unsigned char> Binary = this->ToBinary(TableGeneration);
    std::copy(Binary.cbegin(), Binary.cend(),
        std::ostream_iterator<unsigned char>(File));
    File.close();
}

BymlFile::BymlFile(std::vector<unsigned char> Bytes)
{
    BinaryVectorReader Reader(Bytes);

    char Magic[2]; //Magic, should be YB or BY
    Reader.Read(Magic, 2);
    if ((Magic[0] != 'Y' && Magic[0] != 'B') && (Magic[1] != 'Y' && Magic[1] != 'B'))
    {
        std::cerr << "Expected YB or BY as magic, but got " << Magic << "!\n";
        return;
    }

    uint16_t Version = Reader.ReadUInt16(); //Version should be 7 or 2
    if (Version != 0x07 && Version != 0x02)
    {
        std::cerr << "Expected version 7, but got " << Version << "!\n";
        return;
    }

    uint32_t KeyTableOffset = Reader.ReadUInt32();
    uint32_t StringTableOffset = Reader.ReadUInt32();
    uint32_t DataOffset = Reader.ReadUInt32();

    if (KeyTableOffset != 0) //If equal to 0, there is no Key Table
    {
        this->ParseTable(Reader, &this->m_HashKeyTable, KeyTableOffset);
    }

    if (StringTableOffset != 0) //If equal to 0, there is no String Table
    {
        this->ParseTable(Reader, &this->m_StringTable, StringTableOffset);
    }
    
    /* Parsing actual BYML data */
    Reader.Seek(DataOffset, BinaryVectorReader::Position::Begin);
    uint8_t RootNodeType = Reader.ReadUInt8();
    if (RootNodeType != (uint8_t)BymlFile::Type::Array && RootNodeType != (uint8_t)BymlFile::Type::Dictionary)
    {
        std::cerr << "Expected array (0xC0) or dictionary (0xC1), but got " << (int)RootNodeType << "!\n";
        return;
    }

    BymlFile::Node RootNode(RootNodeType == 0xC0 ? BymlFile::Type::Array : BymlFile::Type::Dictionary, "root");
    
    this->ParseNode(Reader, 12, RootNode.GetType(), "root", &RootNode);

    for (BymlFile::Node Node : RootNode.GetChildren()[0].GetChildren())
    {
        this->m_Nodes.push_back(Node);
    }
    this->m_Type = RootNode.GetType();
}

BymlFile::BymlFile(std::string Path)
{
    std::ifstream File(Path, std::ios::binary);

    if (!File.eof() && !File.fail())
    {
        File.seekg(0, std::ios_base::end);
        std::streampos FileSize = File.tellg();

        std::vector<unsigned char> Bytes(FileSize);

        File.seekg(0, std::ios_base::beg);
        File.read(reinterpret_cast<char*>(Bytes.data()), FileSize);

        this->BymlFile::BymlFile(Bytes);

        File.close();
    }
    else
    {
        std::cerr << "Could not open file \"" << Path << "\"!\n";
    }
}