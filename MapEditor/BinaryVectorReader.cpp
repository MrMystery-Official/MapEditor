#include "BinaryVectorReader.h"

BinaryVectorReader::BinaryVectorReader(std::vector<unsigned char>& Bytes) : m_Bytes(Bytes) {}

void BinaryVectorReader::Seek(int Offset, BinaryVectorReader::Position Position)
{
	if (Position == BinaryVectorReader::Position::Begin)
	{
		this->m_Offset = Offset - 1;
	} 
	else if (Position == BinaryVectorReader::Position::Current)
	{
		this->m_Offset += Offset;
	}
	else
	{
		this->m_Offset = this->m_Bytes.size() - Offset - 2;
	}
}

int BinaryVectorReader::GetPosition()
{
	return this->m_Offset + 1;
}

void BinaryVectorReader::Read(char* Data, int Length)
{
	for (int i = 0; i < Length; i++) {
		this->m_Offset++;
		Data[i] = this->m_Bytes[this->m_Offset];
	}
}

uint8_t BinaryVectorReader::ReadUInt8()
{
	this->m_Offset++;
	return this->m_Bytes[this->m_Offset];
}

int8_t BinaryVectorReader::ReadInt8()
{
	this->m_Offset++;
	return this->m_Bytes[this->m_Offset];
}

uint16_t BinaryVectorReader::ReadUInt16()
{
	this->m_Offset += 2;
	return (static_cast<uint16_t>(this->m_Bytes[this->m_Offset - 1])) | (this->m_Bytes[this->m_Offset] << 8);
}

int16_t BinaryVectorReader::ReadInt16()
{
	this->m_Offset += 2;
	return (static_cast<int16_t>(this->m_Bytes[this->m_Offset - 1])) | (this->m_Bytes[this->m_Offset] << 8);
}

uint32_t BinaryVectorReader::ReadUInt24() {
	this->m_Offset += 3;
	return (static_cast<uint32_t>(this->m_Bytes[this->m_Offset - 2])) |
		(static_cast<uint32_t>(this->m_Bytes[this->m_Offset - 1]) << 8) |
		(static_cast<uint32_t>(this->m_Bytes[this->m_Offset]) << 16);
}

uint32_t BinaryVectorReader::ReadUInt32()
{
	this->m_Offset += 4;
	return (static_cast<uint32_t>(this->m_Bytes[this->m_Offset - 3])) |
		(static_cast<uint32_t>(this->m_Bytes[this->m_Offset - 2]) << 8) |
		(static_cast<uint32_t>(this->m_Bytes[this->m_Offset - 1]) << 16) |
		static_cast<uint32_t>(this->m_Bytes[this->m_Offset] << 24);
}

int32_t BinaryVectorReader::ReadInt32()
{
	this->m_Offset += 4;
	return (static_cast<int32_t>(this->m_Bytes[this->m_Offset - 3])) |
		(static_cast<int32_t>(this->m_Bytes[this->m_Offset - 2]) << 8) |
		(static_cast<int32_t>(this->m_Bytes[this->m_Offset - 1]) << 16) |
		static_cast<int32_t>(this->m_Bytes[this->m_Offset] << 24);
}

uint64_t BinaryVectorReader::ReadUInt64()
{
	this->m_Offset += 8;
	return (static_cast<uint64_t>(this->m_Bytes[this->m_Offset - 7])) |
		(static_cast<uint64_t>(this->m_Bytes[this->m_Offset - 6]) << 8) |
		(static_cast<uint64_t>(this->m_Bytes[this->m_Offset - 5]) << 16) |
		(static_cast<uint64_t>(this->m_Bytes[this->m_Offset - 4]) << 24) |
		(static_cast<uint64_t>(this->m_Bytes[this->m_Offset - 3]) << 32) |
		(static_cast<uint64_t>(this->m_Bytes[this->m_Offset - 2]) << 40) |
		(static_cast<uint64_t>(this->m_Bytes[this->m_Offset - 1]) << 48) |
		(static_cast<uint64_t>(this->m_Bytes[this->m_Offset]) << 56);
}

int64_t BinaryVectorReader::ReadInt64()
{
	this->m_Offset += 8;
	return (static_cast<int64_t>(this->m_Bytes[this->m_Offset - 7])) |
		(static_cast<int64_t>(this->m_Bytes[this->m_Offset - 6]) << 8) |
		(static_cast<int64_t>(this->m_Bytes[this->m_Offset - 5]) << 16) |
		(static_cast<int64_t>(this->m_Bytes[this->m_Offset - 4]) << 24) |
		(static_cast<int64_t>(this->m_Bytes[this->m_Offset - 3]) << 32) |
		(static_cast<int64_t>(this->m_Bytes[this->m_Offset - 2]) << 40) |
		(static_cast<int64_t>(this->m_Bytes[this->m_Offset - 1]) << 48) |
		(static_cast<int64_t>(this->m_Bytes[this->m_Offset]) << 56);
}

float BinaryVectorReader::ReadFloat()
{
	float Result;
	uint32_t Temp = this->ReadUInt32();
	memcpy(&Result, &Temp, sizeof(Result));
	return Result;
}

double BinaryVectorReader::ReadDouble()
{
	double Result;
	uint32_t Temp = this->ReadUInt64();
	memcpy(&Result, &Temp, sizeof(Result));
	return Result;
}