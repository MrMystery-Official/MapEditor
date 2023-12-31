#pragma once

#include <vector>

class BinaryVectorWriter
{
public:

	enum class Position : uint8_t
	{
		Begin = 0,
		Current = 1,
		End = 2
	};

	int GetPosition();
	void WriteByte(char Byte);
	void WriteBytes(const char* Bytes);
	void WriteInteger(int64_t Data, int Size);
	void WriteRawUnsafe(const char* Bytes, int Size);
	void WriteRawUnsafeFixed(const char* Bytes, int Size);
	void Seek(int Offset, BinaryVectorWriter::Position Start);
	std::vector<unsigned char>& GetData();
private:
	std::vector<unsigned char> m_Data;
	int m_Offset = 0;
};