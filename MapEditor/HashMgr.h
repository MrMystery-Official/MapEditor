#pragma once

#include "Util.h"
#include "Byml.h"

namespace HashMgr
{
	struct ArtificialHash
	{
		uint64_t ActorHash;
		uint64_t PhiveHash;
		uint32_t SRTHash;
	};

	extern uint64_t CurrentHash;
	extern uint32_t CurrentSRTHash;

	void TransformHashesInNode(BymlFile::Node& Node, std::map<uint64_t, uint64_t>* Hashes, std::map<uint32_t, uint32_t>* SRTHashes, std::map<uint64_t, uint64_t>* PhiveHashes);
	void AddHash(ArtificialHash Hash);
	ArtificialHash GetArtificialHash(bool Physics);
	std::vector<ArtificialHash>& GetHashes();
	int& GetIndex();
};