#pragma once

#include <string>
#include <vector>
#include "Actor.h"
#include "Byml.h"
#include "ZStdFile.h"
#include "Config.h"
#include "Util.h"
#include "HashMgr.h"
#include "MapConfig.h"

namespace MapLoader
{
	enum class Type : uint8_t
	{
		SmallDungeon = 0,
		MainField = 1
	};

	std::vector<Actor> LoadMap(std::string Key, MapLoader::Type Type);
	void DetectInternalGameVersion();
};