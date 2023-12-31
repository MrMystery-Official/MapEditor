#include "HashMgr.h"

std::vector<HashMgr::ArtificialHash> Hashes;
int Index = -1;

uint64_t HashMgr::CurrentHash = 0;
uint32_t HashMgr::CurrentSRTHash = 0;

void HashMgr::TransformHashesInNode(BymlFile::Node& Node, std::map<uint64_t, uint64_t>* Hashes, std::map<uint32_t, uint32_t>* SRTHashes, std::map<uint64_t, uint64_t>* PhiveHashes) {
	if (Node.GetType() == BymlFile::Type::UInt32)
	{
		uint32_t Value = Node.GetValue<uint32_t>();
		std::map<uint32_t, uint32_t>::iterator SRTHashIter = SRTHashes->find(Value);
		if (SRTHashIter != SRTHashes->end())
			Node.SetValue<uint32_t>(SRTHashIter->second);
	}
	else if (Node.GetType() == BymlFile::Type::UInt64)
	{
		uint64_t Value = Node.GetValue<uint64_t>();
		std::map<uint64_t, uint64_t>::iterator HashIter = Hashes->find(Value);
		if (HashIter != Hashes->end())
		{
			Node.SetValue<uint64_t>(HashIter->second);
			goto ProcessChilds;
		}

		std::map<uint64_t, uint64_t>::iterator PhiveIter = PhiveHashes->find(Value);
		if (PhiveIter != PhiveHashes->end())
		{
			Node.SetValue<uint64_t>(PhiveIter->second);
		}
	}

	ProcessChilds:
	for (BymlFile::Node& Child : Node.GetChildren())
	{
		TransformHashesInNode(Child, Hashes, SRTHashes, PhiveHashes);
	}
}

void HashMgr::AddHash(HashMgr::ArtificialHash Hash)
{
	Hashes.push_back(Hash);
}

HashMgr::ArtificialHash HashMgr::GetArtificialHash(bool Physics)
{
	if (!Physics || Hashes.empty() || Hashes.size() == Index + 1)
	{
		HashMgr::CurrentHash++;
		HashMgr::CurrentSRTHash++;
		return { HashMgr::CurrentHash, HashMgr::CurrentHash, HashMgr::CurrentSRTHash };
	}

	Index++;
	return Hashes[Index];
}

std::vector<HashMgr::ArtificialHash>& HashMgr::GetHashes()
{
	return Hashes;
}

int& HashMgr::GetIndex()
{
	return Index;
}