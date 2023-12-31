#include "Frontend.h"
#include "Config.h"
#include "ZStdFile.h"
#include "MapLoader.h"
#include "EditorConfig.h"

bool LoadEditorConfig()
{
    if (Util::FileExists(Config::GetWorkingDirFile("Editor.conf")))
    {
        EditorConfig::Load();
        return true;
    }
    return false;
}

int main(int, char**)
{
    Config::WorkingDir = std::filesystem::current_path().string() + "/WorkingDir";
    Util::CreateDir(Config::WorkingDir);
    Util::CreateDir(Config::WorkingDir);
    Util::CreateDir(Config::WorkingDir + "/EditorModels");
    Util::CreateDir(Config::WorkingDir + "/Cache");
    Util::CreateDir(Config::WorkingDir + "/Save");
    Util::CreateDir(Config::WorkingDir + "/Save/Banc");
    Util::CreateDir(Config::WorkingDir + "/Save/Banc/SmallDungeon");
    Util::CreateDir(Config::WorkingDir + "/Save/Banc/SmallDungeon/Merged");
    Util::CreateDir(Config::WorkingDir + "/Save/Banc/SmallDungeon/StartPos");
    Util::CreateDir(Config::WorkingDir + "/Save/Banc/MainField");
    Util::CreateDir(Config::WorkingDir + "/Save/Banc/MainField/Merged");
    Util::CreateDir(Config::WorkingDir + "/Save/Banc/MainField/StartPos");

    bool FoundEConf = LoadEditorConfig();

    std::vector<Actor> Actors;

    Frontend::Initialize(FoundEConf, Actors);

    if (FoundEConf)
    {
        MapLoader::DetectInternalGameVersion();
        ZStdFile::Initialize(Config::GetRomFSFile("Pack/ZsDic.pack.zs"));
        ActorModelLibrary::Initialize();
    }

    /*
    std::vector<Actor> Actors = MapLoader::LoadMap("001", MapLoader::Type::SmallDungeon);

    Frontend::SetActors(&Actors);
    */

    while (!Frontend::ShouldWindowClose())
    {
        Frontend::Render();
    }

    Frontend::CleanUp();

    return 0;
}