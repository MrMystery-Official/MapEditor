#include "RESTBL.h"

void ResTableFile::GenerateCrcTable()
{
    this->m_CrcTable.resize(256);
    for (uint32_t i = 0; i < 256; i++)
    {
        uint32_t c = i;
        for (size_t j = 0; j < 8; j++)
        {
            if (c & 1)
            {
                c = 0xEDB88320 ^ (c >> 1);
            }
            else
            {
                c >>= 1;
            }
        }
        this->m_CrcTable[i] = c;
    }
}

uint32_t ResTableFile::GenerateCrc32Hash(std::string Key)
{
    uint32_t CRC = 0xFFFFFFFF;
    const uint8_t* u = reinterpret_cast<const uint8_t*>(Key.c_str());
    for (size_t i = 0; i < Key.length(); ++i)
    {
        CRC = this->m_CrcTable[(CRC ^ u[i]) & 0xFF] ^ (CRC >> 8);
    }
    return CRC ^ 0xFFFFFFFF;
}

uint32_t ResTableFile::GetFileSize(uint32_t Hash)
{
    for (ResTableFile::CrcEntry& CrcEntry : this->m_CrcEntries)
    {
        if (CrcEntry.Hash == Hash)
        {
            return CrcEntry.Size;
        }
    }
    return 0;
}

uint32_t ResTableFile::GetFileSize(std::string Key)
{
    for (ResTableFile::NameEntry& NameEntry : this->m_NameEntries)
    {
        if (NameEntry.Name == Key)
        {
            return NameEntry.Size;
        }
    }
    return this->GetFileSize(this->GenerateCrc32Hash(Key));
}

void ResTableFile::SetFileSize(uint32_t Hash, uint32_t Value)
{
    for (ResTableFile::CrcEntry& CrcEntry : this->m_CrcEntries)
    {
        if (CrcEntry.Hash == Hash)
        {
            CrcEntry.Size = Value;
            return;
        }
    }

    this->m_CrcEntries.push_back({ Hash, Value });
}

void ResTableFile::SetFileSize(std::string Key, uint32_t Value)
{
    for (ResTableFile::NameEntry& NameEntry : this->m_NameEntries)
    {
        if (NameEntry.Name == Key)
        {
            NameEntry.Size = Value;
            return;
        }
    }
    this->SetFileSize(this->GenerateCrc32Hash(Key), Value);
}

ResTableFile::ResTableFile(std::vector<unsigned char> Data)
{
    this->GenerateCrcTable();

    BinaryVectorReader Reader(Data);

    //Checking magic, should be RESTBL
    char Magic[6];
    Reader.Read(Magic, 6);
    if (Magic[0] != 'R' || Magic[1] != 'E' || Magic[2] != 'S' || Magic[3] != 'T' || Magic[4] != 'B' || Magic[5] != 'L') {
        std::cout << "ERROR: Expected RESTBL, got " << Magic << std::endl;
        return;
    }

    this->m_Header.Version = Reader.ReadUInt32();
    this->m_Header.StringBlockSize = Reader.ReadUInt32();
    this->m_Header.CrcMapCount = Reader.ReadUInt32();
    this->m_Header.NameMapCount = Reader.ReadUInt32();

    this->m_CrcEntries.resize(this->m_Header.CrcMapCount);
    for (int i = 0; i < this->m_Header.CrcMapCount; i++)
    {
        this->m_CrcEntries[i] = { Reader.ReadUInt32(), Reader.ReadUInt32()};
    }

    this->m_NameEntries.resize(this->m_Header.NameMapCount);
    for (int i = 0; i < this->m_Header.NameMapCount; i++)
    {
        uint32_t Offset = 22 + (this->m_Header.CrcMapCount * 8) + ((this->m_Header.StringBlockSize + 4) * i);
        Reader.Seek(Offset, BinaryVectorReader::Position::Begin);
        ResTableFile::NameEntry NameEntry;
        for (int j = 0; j < this->m_Header.StringBlockSize; j++)
        {
            char Character = Reader.ReadInt8();
            if (Character != 0x00)
            {
                NameEntry.Name += Character;
            }
        }
        NameEntry.Size = Reader.ReadUInt32();
        this->m_NameEntries[i] = NameEntry;
    }
}

ResTableFile::ResTableFile(std::string Path)
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

        this->ResTableFile::ResTableFile(Bytes);
    }
    else
    {
        std::cerr << "Could not open file \"" << Path << "\"!\n";
    }
}

std::vector<unsigned char> ResTableFile::ToBinary()
{
    BinaryVectorWriter Writer;

    Writer.WriteBytes("RESTBL");
    Writer.WriteInteger(this->m_Header.Version, sizeof(uint32_t)); //Version
    Writer.WriteInteger(this->m_Header.StringBlockSize, sizeof(uint32_t)); //StringBlockSize
    Writer.WriteInteger(this->m_CrcEntries.size(), sizeof(uint32_t)); //CrcMapCount
    Writer.WriteInteger(this->m_NameEntries.size(), sizeof(uint32_t)); //NameMapCount

    std::sort(this->m_CrcEntries.begin(), this->m_CrcEntries.end(), [](ResTableFile::CrcEntry A, ResTableFile::CrcEntry B) { return A.Hash < B.Hash; });

    for (ResTableFile::CrcEntry CrcEntry : this->m_CrcEntries)
    {
        Writer.WriteInteger(CrcEntry.Hash, sizeof(uint32_t));
        Writer.WriteInteger(CrcEntry.Size, sizeof(uint32_t));
    }

    for (ResTableFile::NameEntry NameEntry : this->m_NameEntries)
    {
        Writer.WriteBytes(NameEntry.Name.c_str());
        Writer.Seek(this->m_Header.StringBlockSize - NameEntry.Name.length(), BinaryVectorWriter::Position::Current);
        Writer.WriteInteger(NameEntry.Size, sizeof(uint32_t));
    }

    return Writer.GetData();
}

void ResTableFile::WriteToFile(std::string Path)
{
    std::ofstream File(Path, std::ios::binary);
    std::vector<unsigned char> Binary = this->ToBinary();
    std::copy(Binary.begin(), Binary.end(),
        std::ostream_iterator<unsigned char>(File));
    File.close();
}