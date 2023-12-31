#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <map>

#include "BinaryVectorReader.h"
#include "BinaryVectorWriter.h"

class BymlFile
{
public:
    /* 
    Node caching:
    Node caching is pretty slow. When enabled, every Dictionary gets saved and compared when rewritten. If it is the same, it doesnt write the Dict, it only writes the
    offset to the first Dict. This produces smaller BYMLs, but takes a lot of time. The game normally reads a BYML file even without node caching, but for large system files it is needed.
    (e.g. ActorInfo)
    */
    bool NodeCaching = false;

	enum class Type : unsigned char
	{
        StringIndex = 0xa0,
        Array = 0xc0,
        Dictionary = 0xc1,
        KeyTable = 0xc2,
        Bool = 0xd0,
        Int32 = 0xd1,
        Float = 0xd2,
        UInt32 = 0xd3,
        Int64 = 0xd4,
        UInt64 = 0xd5,
        Double = 0xd6,
        Null = 0xff
	};

    enum class TableGeneration : unsigned char
    {
        Auto = 0,
        Manual = 1
    };

	class Node
	{
    public:
        Node(BymlFile::Type Type, std::string Key = "");

        BymlFile::Type& GetType();
        std::string& GetKey();
        template<class T> T GetValue();
        template<class T> void SetValue(T Value);

        bool HasChild(std::string Key);
        BymlFile::Node* GetChild(std::string Key);
        BymlFile::Node* GetChild(int Index);
        void AddChild(BymlFile::Node& Node);
        std::vector<BymlFile::Node>& GetChildren();
    private:
        BymlFile::Type m_Type;
        std::string m_Key;
        std::vector<unsigned char> m_Value;

        std::vector<BymlFile::Node> m_Children;
	};

	BymlFile(std::string Path);
	BymlFile(std::vector<unsigned char> Bytes);
    BymlFile() {}

    std::vector<BymlFile::Node>& GetNodes();
    BymlFile::Type& GetType();
    BymlFile::Node* GetNode(std::string Key);
    bool HasChild(std::string Key);

    void AddHashKeyTableEntry(std::string Key);
    void AddStringTableEntry(std::string String);

    void GenerateHashKeyTable(BymlFile::Node* Node);
    void GenerateStringTable(BymlFile::Node* Node);

    std::vector<unsigned char> ToBinary(BymlFile::TableGeneration TableGeneration);
    void WriteToFile(std::string Path, BymlFile::TableGeneration TableGeneration);
private:
    std::vector<BymlFile::Node> m_Nodes;
    std::vector<std::string> m_HashKeyTable;
    std::vector<std::string> m_StringTable;
    BymlFile::Type m_Type = BymlFile::Type::Null;
    uint32_t m_WriterLastOffset = 0;
    uint32_t m_WriterReservedDataOffset = 0;
    std::map<std::string, uint32_t> m_CachedNodes;

    void ParseTable(BinaryVectorReader& Reader, std::vector<std::string>* Dest, int TableOffset);
    int AlignUp(int Value, int Size);
    bool IsNumber(const std::string& str);
    uint32_t GetStringTableIndex(std::string Value);
    uint32_t GetHashKeyTableIndex(std::string Value);
    void ParseNode(BinaryVectorReader& Reader, int Offset, BymlFile::Type Type, std::string Key, BymlFile::Node* Parent);
    std::string GenerateNodeHash(BymlFile::Node* Node);
    void WriteNode(BinaryVectorWriter& Writer, uint32_t DataOffset, uint32_t Offset, BymlFile::Node* Node);
};