#include "ZStdFile.h"

ZSTD_DDict* m_DecompressDictBase;
ZSTD_DDict* m_DecompressDictPack;
ZSTD_DDict* m_DecompressDictBcettByaml;

ZSTD_CDict* m_CompressDictBase;
ZSTD_CDict* m_CompressDictPack;
ZSTD_CDict* m_CompressDictBcettByaml;

void ZStdFile::Result::WriteToFile(std::string Path)
{
	std::ofstream File(Path, std::ios::binary);
	std::copy(this->Data.begin(), this->Data.end(),
		std::ostream_iterator<unsigned char>(File));
	File.close();
}

void ZStdFile::Initialize(std::string Path)
{
	SarcFile DictSarc(ZStdFile::Decompress(Path, ZStdFile::Dictionary::None).Data);

	std::vector<unsigned char>* DictPack = &DictSarc.GetEntry("pack.zsdic").Bytes;
	std::vector<unsigned char>* DictBase = &DictSarc.GetEntry("zs.zsdic").Bytes;
	std::vector<unsigned char>* DictBcettByaml = &DictSarc.GetEntry("bcett.byml.zsdic").Bytes;

	m_DecompressDictBase = ZSTD_createDDict(DictBase->data(), DictBase->size());
	m_DecompressDictPack = ZSTD_createDDict(DictPack->data(), DictPack->size());
	m_DecompressDictBcettByaml = ZSTD_createDDict(DictBcettByaml->data(), DictBcettByaml->size());

	//16 = compression level
	m_CompressDictBase = ZSTD_createCDict(DictBase->data(), DictBase->size(), 16);
	m_CompressDictPack = ZSTD_createCDict(DictPack->data(), DictPack->size(), 16);
	m_CompressDictBcettByaml = ZSTD_createCDict(DictBcettByaml->data(), DictBcettByaml->size(), 16);
}

int ZStdFile::GetDecompressedFileSize(std::string Path, ZStdFile::Dictionary Dictionary)
{
	std::ifstream File(Path, std::ios::binary);

	if (!File.eof() && !File.fail())
	{
		File.seekg(0, std::ios_base::end);
		std::streampos FileSize = File.tellg();

		std::vector<unsigned char> Bytes(FileSize);

		File.seekg(0, std::ios_base::beg);
		File.read(reinterpret_cast<char*>(Bytes.data()), FileSize);

		File.close();

		unsigned long long DecompSize = ZSTD_getFrameContentSize(Bytes.data(), Bytes.size());
		if (DecompSize == 18446744073709551614) //Means the size could not be calculated
		{
			return -1;
		}

		return DecompSize;
	}
	else
	{
		std::cerr << "Could not open file \"" << Path << "\"!\n";
		return -1;
	}
}

ZStdFile::Result ZStdFile::Decompress(std::vector<unsigned char> Bytes, ZStdFile::Dictionary Dictionary)
{
	ZStdFile::Result Result;

	unsigned long long DecompSize = ZSTD_getFrameContentSize(Bytes.data(), Bytes.size());
	if (DecompSize == 18446744073709551614) //Means the size could not be calculated
	{
		return Result;
	}
	Result.Data.resize(DecompSize);

	if (Dictionary == ZStdFile::Dictionary::None)
	{
		ZSTD_DCtx* const DCtx = ZSTD_createDCtx();

		const size_t DecompSize = ZSTD_decompressDCtx(DCtx, (void*)Result.Data.data(), Result.Data.size(), Bytes.data(), Bytes.size());
		Result.Data.resize(DecompSize);

		ZSTD_freeDCtx(DCtx);
	}
	else
	{
		ZSTD_DDict* DecompressionDict = nullptr;
		switch (Dictionary)
		{
		case ZStdFile::Dictionary::Base:
			DecompressionDict = m_DecompressDictBase;
			break;
		case ZStdFile::Dictionary::Pack:
			DecompressionDict = m_DecompressDictPack;
			break;
		case ZStdFile::Dictionary::BcettByaml:
			DecompressionDict = m_DecompressDictBcettByaml;
			break;
		}

		unsigned const ExpectedDictID = ZSTD_getDictID_fromDDict(DecompressionDict);
		unsigned const ActualDictID = ZSTD_getDictID_fromFrame(Bytes.data(), Bytes.size());
		if (ExpectedDictID != ActualDictID)
		{
			std::cerr << "ZStd Dictionary mismatch! Expected the dictionary with ID " << ExpectedDictID << ", but data has to be decompressed with dictionary " << ActualDictID << "!\n";
			return ZStdFile::Result();
		}

		ZSTD_DCtx* const DCtx = ZSTD_createDCtx();
		const size_t DecompSize = ZSTD_decompress_usingDDict(DCtx, (void*)Result.Data.data(), Result.Data.size(), Bytes.data(), Bytes.size(), DecompressionDict);
		Result.Data.resize(DecompSize);

		ZSTD_freeDCtx(DCtx);
	}

	return Result;
}

ZStdFile::Result ZStdFile::Decompress(std::string Path, ZStdFile::Dictionary Dictionary)
{
	std::ifstream File(Path, std::ios::binary);

	if (!File.eof() && !File.fail())
	{
		File.seekg(0, std::ios_base::end);
		std::streampos FileSize = File.tellg();

		std::vector<unsigned char> Bytes(FileSize);

		File.seekg(0, std::ios_base::beg);
		File.read(reinterpret_cast<char*>(Bytes.data()), FileSize);

		File.close();

		return ZStdFile::Decompress(Bytes, Dictionary);
	}
	else
	{
		std::cerr << "Could not open file \"" << Path << "\"!\n";
		return ZStdFile::Result();
	}
}

ZStdFile::Result ZStdFile::Compress(std::vector<unsigned char> Bytes, ZStdFile::Dictionary Dictionary)
{
	ZStdFile::Result Result;
	Result.Data.resize(ZSTD_compressBound(Bytes.size()));

	if (Dictionary == ZStdFile::Dictionary::None)
	{
		ZSTD_CCtx* cctx = ZSTD_createCCtx();
		ZSTD_CCtx_setParameter(cctx, ZSTD_c_compressionLevel, 16); //16 = compression level
		const size_t CompSize = ZSTD_compressCCtx(cctx, (void*)Result.Data.data(), Result.Data.size(), Bytes.data(), Bytes.size(), 16);
		Result.Data.resize(CompSize);
		ZSTD_freeCCtx(cctx);
	}
	else
	{
		ZSTD_CDict* CompressionDict = nullptr;
		switch (Dictionary)
		{
		case ZStdFile::Dictionary::Base:
			CompressionDict = m_CompressDictBase;
			break;
		case ZStdFile::Dictionary::Pack:
			CompressionDict = m_CompressDictPack;
			break;
		case ZStdFile::Dictionary::BcettByaml:
			CompressionDict = m_CompressDictBcettByaml;
			break;
		}

		ZSTD_CCtx* cctx = ZSTD_createCCtx();
		ZSTD_CCtx_setParameter(cctx, ZSTD_c_compressionLevel, 16); //16 = compression level
		const size_t CompSize = ZSTD_compress_usingCDict(cctx, (void*)Result.Data.data(), Result.Data.size(), Bytes.data(), Bytes.size(), CompressionDict);
		Result.Data.resize(CompSize);
		ZSTD_freeCCtx(cctx);
	}

	return Result;
}