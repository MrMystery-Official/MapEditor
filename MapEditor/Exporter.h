#pragma once

#include <string>
#include "Config.h"
#include "Actor.h"
#include "Util.h"
#include "ZStdFile.h"
#include "MapConfig.h"
#include "RESTBL.h"
#include "EditorConfig.h"

namespace Exporter
{
	BymlFile::Node ActorToByml(std::string Path, bool CreateCollisionActors, std::vector<Actor>* Actors, Actor& ExportedActor);
	void Export(std::vector<Actor>* Actors, std::string Path, bool Save);
};