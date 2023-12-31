#pragma once

#include <vector>
#include <iostream>
#include <zstd.h>

#include "SARC.h"

namespace ZStdFile
{
	void Initialize(std::string Path); //Load the TotK ZStandard dictionaries (requires SARC)

	enum class Dictionary
	{
		None = -1,
		Base = 0,
		Pack = 1,
		BcettByaml = 2
	};

	struct Result
	{
		std::vector<unsigned char> Data;

		void WriteToFile(std::string Path);
	};

	int GetDecompressedFileSize(std::string Path, ZStdFile::Dictionary Dictionary);

	ZStdFile::Result Decompress(std::vector<unsigned char> Bytes, ZStdFile::Dictionary Dictionary);
	ZStdFile::Result Decompress(std::string Path, ZStdFile::Dictionary Dictionary);

	ZStdFile::Result Compress(std::vector<unsigned char> Bytes, ZStdFile::Dictionary Dictionary);
};