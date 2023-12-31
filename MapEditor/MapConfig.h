#pragma once

#include "BinaryVectorReader.h"
#include "BinaryVectorWriter.h"
#include "HashMgr.h"
#include "Config.h"
#include "Actor.h"

namespace MapConfig
{
	void Load(std::vector<Actor>* Actors);
	void Save(std::vector<Actor>* Actors);
};