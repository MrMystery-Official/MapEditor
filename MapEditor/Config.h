#pragma once

#include <string>
#include <imgui.h>
#include <map>
#include "Byml.h"
#include "Vector3F.h"
#include "Actor.h"

namespace Config
{
	extern BymlFile StaticActorsByml;
	extern BymlFile DynamicActorsByml;
	extern std::map<uint32_t, BymlFile> MergedActorBymls;
	extern std::string Key;
	extern uint8_t MapType;
	extern std::string WorkingDir;
	extern std::string BancPrefix;

	extern std::string RomFSPath;
	extern std::string BfresPath;
	extern std::string InternalGameVersion;
	extern std::string ExportPath;

	extern float UIScale;
	extern std::map<float, ImFont*>* Fonts;

	std::string GetRomFSFile(std::string LocalPath, bool Replaceable = true);
	std::string GetBfresFile(std::string Name);
	std::string GetTextureFile(std::string TextureName);
	std::string GetWorkingDirFile(std::string Path);
	std::string GetInternalGameVersion();
};