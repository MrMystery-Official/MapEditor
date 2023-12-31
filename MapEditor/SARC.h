#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <algorithm>

#include "BinaryVectorReader.h"
#include "BinaryVectorWriter.h"

class SarcFile
{
public:
	struct Entry
	{
		std::string Name;
		std::vector<unsigned char> Bytes;

		void WriteToFile(std::string Path);
	};

	std::vector<SarcFile::Entry>& GetEntries();
	SarcFile::Entry& GetEntry(std::string Name);

	std::vector<unsigned char> ToBinary();
	void WriteToFile(std::string Path);

	SarcFile(std::string Path);
	SarcFile(std::vector<unsigned char> Bytes);
private:
	struct Node
	{
		uint32_t Hash;
		uint32_t StringOffset;
		uint32_t DataStart;
		uint32_t DataEnd;
	};

	struct HashValue
	{
		uint32_t Hash = 0;
		SarcFile::Entry* Node = nullptr;
	};

	std::vector<Entry> m_Entries;

	int GCD(int a, int b);
	int LCM(int a, int b);
	int AlignUp(int Value, int Size);
	int GetBinaryFileAlignment(std::vector<unsigned char> Data);
};