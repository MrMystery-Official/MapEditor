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
	void Export(std::vector<Actor>* Actors, std::string Path, bool Save);
};