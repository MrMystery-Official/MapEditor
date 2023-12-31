#pragma once

#include <vector>

class BinaryVectorReader
{
public:
	enum class Position : uint8_t
	{
		Begin = 0,
		Current = 1,
		End = 2
	};

	BinaryVectorReader(std::vector<unsigned char>& Bytes);

	void Seek(int Offset, BinaryVectorReader::Position Position);
	int GetPosition();
	void Read(char* Data, int Length);
	uint8_t ReadUInt8();
	int8_t ReadInt8();
	uint16_t ReadUInt16();
	int16_t ReadInt16();
	uint32_t ReadUInt24();
	uint32_t ReadUInt32();
	int32_t ReadInt32();
	uint64_t ReadUInt64();
	int64_t ReadInt64();
	float ReadFloat();
	double ReadDouble();
private:
	std::vector<unsigned char>& m_Bytes;
	int m_Offset = -1;
};