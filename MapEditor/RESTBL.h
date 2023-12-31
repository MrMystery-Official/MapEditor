#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>

#include "BinaryVectorReader.h"
#include "BinaryVectorWriter.h"

class ResTableFile
{
public:
	uint32_t GetFileSize(uint32_t Hash);
	uint32_t GetFileSize(std::string Key);
	void SetFileSize(uint32_t Hash, uint32_t Value);
	void SetFileSize(std::string Key, uint32_t Value);

	uint32_t GenerateCrc32Hash(std::string Key);
	std::vector<unsigned char> ToBinary();
	void WriteToFile(std::string Path);

	ResTableFile(std::vector<unsigned char> Data);
	ResTableFile(std::string Path);
private:
	struct Header
	{
		uint32_t Version;
		uint32_t StringBlockSize;
		uint32_t CrcMapCount;
		uint32_t NameMapCount;
	};

	struct CrcEntry
	{
		uint32_t Hash;
		uint32_t Size;
	};

	struct NameEntry
	{
		std::string Name;
		uint32_t Size;
	};

	std::vector<uint32_t> m_CrcTable;
	std::vector<ResTableFile::CrcEntry> m_CrcEntries;
	std::vector<ResTableFile::NameEntry> m_NameEntries;
	ResTableFile::Header m_Header;

	void GenerateCrcTable();
};