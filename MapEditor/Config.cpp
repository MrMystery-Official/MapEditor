#include "Config.h"
#include "Util.h"

BymlFile Config::StaticActorsByml;
BymlFile Config::DynamicActorsByml;
std::map<uint32_t, BymlFile> Config::MergedActorBymls;
std::string Config::Key = "";
uint8_t Config::MapType;
std::string Config::WorkingDir;
std::string Config::BancPrefix;

std::string Config::RomFSPath;
std::string Config::BfresPath;
std::string Config::InternalGameVersion = "100";
std::string Config::ExportPath = "";

float Config::UIScale = 0;
std::map<float, ImFont*>* Config::Fonts = new std::map<float, ImFont*>();

std::string Config::GetWorkingDirFile(std::string Path)
{
	return WorkingDir + "/" + Path;
}

std::string Config::GetRomFSFile(std::string LocalPath, bool Replaceable)
{
	if (Util::FileExists(Config::GetWorkingDirFile("Save/" + LocalPath)) && Replaceable)
	{
		return Config::GetWorkingDirFile("Save/" + LocalPath);
	}

	return Config::RomFSPath + "/" + LocalPath;
}

std::string Config::GetTextureFile(std::string TextureName)
{
	if (Util::FileExists(Config::GetWorkingDirFile("Save/TexToGo/" + TextureName)))
	{
		return Config::GetWorkingDirFile("Save/TexToGo/" + TextureName);
	}

	return Config::RomFSPath + "/TexToGo/" + TextureName;
}

std::string Config::GetBfresFile(std::string Name)
{
	if (Util::FileExists(Config::GetWorkingDirFile("EditorModels/" + Name)))
	{
		return Config::GetWorkingDirFile("EditorModels/" + Name);
	}

	return Config::BfresPath + "/" + Name;
}

std::string Config::GetInternalGameVersion()
{
	return Config::InternalGameVersion;
}