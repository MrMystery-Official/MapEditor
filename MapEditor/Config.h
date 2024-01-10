#pragma once

#include <string>
#include "Byml.h"

namespace Config
{
	extern BymlFile StaticActorsByml;
	extern BymlFile DynamicActorsByml;
	extern std::string Key;
	extern uint8_t MapType;
	extern std::string WorkingDir;
	extern std::string BancPrefix;

	extern std::string RomFSPath;
	extern std::string BfresPath;
	extern std::string InternalGameVersion;
	extern std::string ExportPath;

	extern float UIScale;

	std::string GetRomFSFile(std::string LocalPath, bool Replaceable = true);
	std::string GetBfresFile(std::string Name);
	std::string GetTextureFile(std::string TextureName);
	std::string GetWorkingDirFile(std::string Path);
	std::string GetInternalGameVersion();
};