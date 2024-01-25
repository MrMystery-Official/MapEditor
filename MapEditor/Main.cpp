#include "Frontend.h"
#include "Config.h"
#include "ZStdFile.h"
#include "MapLoader.h"
#include "EditorConfig.h"
#include "AINB.h"
#include "AINBNodeDefinitions.h"
#include "TemplateMgr.h"

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
    Util::CreateDir(Config::WorkingDir + "/Templates");
    Util::CreateDir(Config::WorkingDir + "/EditorModels");
    Util::CreateDir(Config::WorkingDir + "/Cache");
    Util::CreateDir(Config::WorkingDir + "/Save");
    Util::CreateDir(Config::WorkingDir + "/Save/Logic");
    Util::CreateDir(Config::WorkingDir + "/Save/Banc");
    Util::CreateDir(Config::WorkingDir + "/Save/Banc/SmallDungeon");
    Util::CreateDir(Config::WorkingDir + "/Save/Banc/SmallDungeon/Merged");
    Util::CreateDir(Config::WorkingDir + "/Save/Banc/LargeDungeon");
    Util::CreateDir(Config::WorkingDir + "/Save/Banc/LargeDungeon/Merged");
    Util::CreateDir(Config::WorkingDir + "/Save/Banc/MainField");
    Util::CreateDir(Config::WorkingDir + "/Save/Banc/MainField/Merged");
    Util::CreateDir(Config::WorkingDir + "/Save/Banc/MainField/Sky");
    Util::CreateDir(Config::WorkingDir + "/Save/Banc/MinusField");
    Util::CreateDir(Config::WorkingDir + "/Save/Banc/MinusField/Merged");
    Util::CreateDir(Config::WorkingDir + "/Save/Banc/NormalStage");
    Util::CreateDir(Config::WorkingDir + "/Save/Banc/NormalStage/Merged");

    bool FoundEConf = LoadEditorConfig();

    std::vector<Actor> Actors;

    Frontend::Initialize(FoundEConf, Actors);

    if (FoundEConf)
    {
        MapLoader::DetectInternalGameVersion();
        ZStdFile::Initialize(Config::GetRomFSFile("Pack/ZsDic.pack.zs"));
        ActorModelLibrary::Initialize();
        //AINBNodeDefinitions::Generate();
        AINBNodeDefinitions::Initialize();
        TemplateMgr::Initialize();
    }

    while (!Frontend::ShouldWindowClose())
    {
        Frontend::Render();
    }

    Frontend::CleanUp();

    return 0;
}