#include "MapConfig.h"

void MapConfig::Load(std::vector<Actor>* Actors)
{
		std::cout << "STREAM\n";
		std::ifstream File(Config::GetWorkingDirFile("Save/Map_" + std::to_string((int)Config::MapType) + "_" + Config::Key + ".conf"), std::ios::binary);

		if (!File.eof() && !File.fail())
		{
			File.seekg(0, std::ios_base::end);
			std::streampos FileSize = File.tellg();

			std::vector<unsigned char> Bytes(FileSize);

			File.seekg(0, std::ios_base::beg);
			File.read(reinterpret_cast<char*>(Bytes.data()), FileSize);

			File.close();

			BinaryVectorReader Reader(Bytes);

			Reader.Seek(4, BinaryVectorReader::Position::Current);
			uint8_t Version = Reader.ReadUInt8();
			if (Version != 0x01)
			{
				std::cerr << "Expected version 0x01, but got " << (int)Version << "!\n";
				return;
			}

			HashMgr::CurrentHash = Reader.ReadUInt64();
			HashMgr::CurrentSRTHash = Reader.ReadUInt32();

			uint32_t HashesSize = Reader.ReadUInt32();
			HashMgr::GetIndex() = Reader.ReadInt32();

			uint32_t ClimbableCollisionCount = Reader.ReadUInt32();
			for (int i = 0; i < ClimbableCollisionCount; i++)
			{
				uint32_t SRTHash = Reader.ReadUInt32();
				for (Actor& Actor : *Actors)
				{
					if (Actor.GetSRTHash() == SRTHash)
					{
						Actor.SetCollisionClimbable(true);
						break;
					}
				}
			}

			uint32_t FileCollisionCount = Reader.ReadUInt32();
			for (int i = 0; i < FileCollisionCount; i++)
			{
				uint32_t FileLength = Reader.ReadUInt32();
				uint32_t SRTHash = Reader.ReadUInt32();
				std::string FilePath = "";
				for (int i = 0; i < FileLength; i++)
				{
					FilePath += (char)Reader.ReadInt8();
				}
				for (Actor& Actor : *Actors)
				{
					if (Actor.GetSRTHash() == SRTHash)
					{
						Actor.SetCollisionFile(FilePath);
						break;
					}
				}
			}

			for (int i = 0; i < HashesSize; i++)
			{
				HashMgr::AddHash({ Reader.ReadUInt64(), Reader.ReadUInt64(), Reader.ReadUInt32() });
			}

			std::cout << "LOAD\n";
		}
}

void MapConfig::Save(std::vector<Actor>* Actors)
{
	BinaryVectorWriter Writer;
	Writer.WriteBytes("MAPC"); //Magic
	Writer.WriteByte(0x01); //Version 1
	Writer.WriteInteger(HashMgr::CurrentHash, sizeof(uint64_t));
	Writer.WriteInteger(HashMgr::CurrentSRTHash, sizeof(uint32_t));

	Writer.WriteInteger(HashMgr::GetHashes().size(), sizeof(uint32_t));
	Writer.WriteInteger(HashMgr::GetIndex(), sizeof(int32_t));

	uint32_t ClimbableCollisionCount = 0;
	uint32_t FileCollisionCount = 0;
	Writer.WriteInteger(0, sizeof(uint32_t)); //CollisionClimbable Placeholder
	for (Actor& Actor : *Actors)
	{
		if (Actor.IsCollisionClimbable())
		{
			ClimbableCollisionCount++;

			Writer.WriteInteger(Actor.GetSRTHash(), sizeof(uint32_t));
		}
		if (Actor.GetCollisionFile() != "")
		{
			FileCollisionCount++;
		}
	}
	Writer.Seek(25, BinaryVectorWriter::Position::Begin);
	Writer.WriteInteger(ClimbableCollisionCount, sizeof(uint32_t));
	Writer.Seek(29 + (ClimbableCollisionCount * 4), BinaryVectorWriter::Position::Begin);

	Writer.WriteInteger(FileCollisionCount, sizeof(uint32_t)); //CollisionClimbable Placeholder
	for (Actor& Actor : *Actors)
	{
		if (Actor.GetCollisionFile() != "")
		{
			Writer.WriteInteger(Actor.GetCollisionFile().length(), sizeof(uint32_t));
			Writer.WriteInteger(Actor.GetSRTHash(), sizeof(uint32_t));
			Writer.WriteBytes(Actor.GetCollisionFile().c_str());
		}
	}

	for (const HashMgr::ArtificialHash& Hash : HashMgr::GetHashes())
	{
		Writer.WriteInteger(Hash.ActorHash, sizeof(uint64_t));
		Writer.WriteInteger(Hash.PhiveHash, sizeof(uint64_t));
		Writer.WriteInteger(Hash.SRTHash, sizeof(uint32_t));
	}

	std::ofstream File(Config::GetWorkingDirFile("Save/Map_" + std::to_string((int)Config::MapType) + "_" + Config::Key + ".conf"), std::ios::binary);
	std::copy(Writer.GetData().cbegin(), Writer.GetData().cend(),
		std::ostream_iterator<unsigned char>(File));
	File.close();
}