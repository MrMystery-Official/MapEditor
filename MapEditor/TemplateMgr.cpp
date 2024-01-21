#include "TemplateMgr.h"

#include "BinaryVectorReader.h"
#include "BinaryVectorWriter.h"
#include "Actor.h"
#include "MapLoader.h"
#include "Exporter.h"
#include "HashMgr.h"

std::vector<TemplateMgr::Template> TemplateMgr::Templates;

/*
	class Template
	{
	public:
		std::string& GetName();
		std::vector<Actor>& GetActors();

		void Paste(Vector3F BaseLocation, std::vector<Actor>* Actors);
	private:
		std::string m_Name;
		std::vector<Actor> m_Actors;
	};
*/

TemplateMgr::Template::Template(std::string Name) : m_Name(Name)
{
}

TemplateMgr::Template::Template(std::string Name, std::vector<Actor> Actors) : m_Name(Name), m_Actors(Actors)
{
}


std::string& TemplateMgr::Template::GetName()
{
	return this->m_Name;
}

std::vector<Actor>& TemplateMgr::Template::GetActors()
{
	return this->m_Actors;
}

void TemplateMgr::Template::Paste(Vector3F BaseLocation, std::vector<Actor>* Actors)
{
	std::map<uint64_t, HashMgr::ArtificialHash> NewHashes;

	for (Actor& TmplActor : this->m_Actors)
	{
		NewHashes.insert({ TmplActor.GetHash(), HashMgr::GetArtificialHash(false) });
	}

	for (Actor TmplActor : this->m_Actors)
	{
		TmplActor.GetTranslate().SetX(TmplActor.GetTranslate().GetX() + BaseLocation.GetX());
		TmplActor.GetTranslate().SetY(TmplActor.GetTranslate().GetY() + BaseLocation.GetY());
		TmplActor.GetTranslate().SetZ(TmplActor.GetTranslate().GetZ() + BaseLocation.GetZ());

		TmplActor.SetSRTHash(NewHashes[TmplActor.GetHash()].SRTHash);

		if (TmplActor.GetPhive().Placement.count("ID"))
		{
			TmplActor.GetPhive().Placement["ID"] = std::to_string(NewHashes[TmplActor.GetHash()].ActorHash);
		}

		std::vector<Actor::Link>::iterator Iter;
		for (Iter = TmplActor.GetLinks().begin(); Iter != TmplActor.GetLinks().end(); ) {
			if (NewHashes.count(Iter->Src))
				Iter->Src = NewHashes[Iter->Src].ActorHash;

			if (NewHashes.count(Iter->Dst))
			{
				Iter->Dst = NewHashes[Iter->Dst].ActorHash;
				++Iter;
			}
			else
			{
				Iter = TmplActor.GetLinks().erase(Iter);
			}
		}

		TmplActor.SetHash(NewHashes[TmplActor.GetHash()].ActorHash);

		if (TmplActor.GetGyml().find("Area") != std::string::npos)
		{
			if (TmplActor.GetGyml().find("Forbid") == std::string::npos)
			{
				TmplActor.SetModel(ActorModelLibrary::GetModel("Area"));
			}
			else
			{
				TmplActor.SetModel(ActorModelLibrary::GetModel("ForbidArea"));
			}
			TmplActor.SetCategory("System");
			goto FinishedBfresLoading;
		}

		if (TmplActor.GetGyml().find("MapEditor_Collision_") != std::string::npos)
		{
			TmplActor.SetModel(ActorModelLibrary::GetModel("Collision"));
			TmplActor.SetCategory("Editor");
			goto FinishedBfresLoading;
		}

		{
			SarcFile ActorPackFile(ZStdFile::Decompress(Config::GetRomFSFile("Pack/Actor/" + TmplActor.GetGyml() + ".pack.zs"), ZStdFile::Dictionary::Pack).Data);
			std::string ModelInfoEntryName;
			for (SarcFile::Entry Entry : ActorPackFile.GetEntries())
			{
				if (Entry.Name.rfind("Component/ModelInfo/", 0) == 0)
				{
					ModelInfoEntryName = Entry.Name;
					break;
				}
			}
			if (ModelInfoEntryName.rfind("Component/ModelInfo/None", 0) == 0 || ModelInfoEntryName == "")
			{
				TmplActor.SetModel(ActorModelLibrary::GetModel("None"));
				std::cout << "Warning: Could not find a model for actor " << TmplActor.GetGyml() << "!\n";
			}
			else
			{
				BymlFile ModelInfo(ActorPackFile.GetEntry(ModelInfoEntryName).Bytes);
				if (ModelInfo.GetNodes().size() == 0)
				{
					TmplActor.SetModel(ActorModelLibrary::GetModel("None"));
					goto FinishedBfresLoading;
				}
				TmplActor.SetModel(ActorModelLibrary::GetModel(ModelInfo.GetNode("ModelProjectName")->GetValue<std::string>() + "." + ModelInfo.GetNode("FmdbName")->GetValue<std::string>()));
			}

		}

		FinishedBfresLoading:
		Actors->push_back(TmplActor);
	}
}

void TemplateMgr::Initialize()
{
	for (const auto& Entry : std::filesystem::directory_iterator(Config::GetWorkingDirFile("Templates")))
	{
		if (Entry.is_regular_file())
		{
			BymlFile File(Entry.path().string());

			TemplateMgr::Template Tmpl(File.GetNode("Name")->GetValue<std::string>());
			Tmpl.GetActors().resize(File.GetNode("Actors")->GetChildren().size());

			for (int i = 0; i < Tmpl.GetActors().size(); i++)
			{
				Actor TmplActor;
				TmplActor.SetType(static_cast<Actor::Type>(File.GetNode("Actors")->GetChild(i)->GetChild("BymlType")->GetValue<uint32_t>()));
				TmplActor.SetCollisionClimbable(File.GetNode("Actors")->GetChild(i)->GetChild("CollisionClimbable")->GetValue<bool>());
				TmplActor.SetCollisionFile(File.GetNode("Actors")->GetChild(i)->GetChild("CollisionFile")->GetValue<std::string>());
				MapLoader::InterpretActorNode(&TmplActor, File.GetNode("Actors")->GetChild(i));
				Tmpl.GetActors()[i] = TmplActor;
			}
			TemplateMgr::Templates.push_back(Tmpl);
		}
	}
}

void TemplateMgr::Save()
{
	for (TemplateMgr::Template& Tmpl : TemplateMgr::Templates)
	{
		BymlFile File;
		File.GetType() = BymlFile::Type::Dictionary;

		BymlFile::Node NameNode(BymlFile::Type::StringIndex, "Name");
		NameNode.SetValue<std::string>(Tmpl.GetName());

		BymlFile::Node ActorsNode(BymlFile::Type::Array, "Actors");

		for (Actor& TmplActor : Tmpl.GetActors())
		{
			BymlFile::Node ActorNode = Exporter::ActorToByml("", false, &Tmpl.GetActors(), TmplActor);

			BymlFile::Node ActorBymlType(BymlFile::Type::UInt32, "BymlType");
			ActorBymlType.SetValue<uint32_t>((uint32_t)TmplActor.GetType());

			BymlFile::Node ActorCollisionClimbable(BymlFile::Type::Bool, "CollisionClimbable");
			ActorCollisionClimbable.SetValue<bool>(TmplActor.IsCollisionClimbable());

			BymlFile::Node ActorCollisionFile(BymlFile::Type::StringIndex, "CollisionFile");
			ActorCollisionFile.SetValue<std::string>(TmplActor.GetCollisionFile());

			ActorNode.AddChild(ActorBymlType);
			ActorNode.AddChild(ActorCollisionClimbable);
			ActorNode.AddChild(ActorCollisionFile);
			ActorsNode.AddChild(ActorNode);
		}

		File.GetNodes().push_back(NameNode);
		File.GetNodes().push_back(ActorsNode);
		File.WriteToFile(Config::GetWorkingDirFile("Templates/" + Tmpl.GetName() + ".tmpl.byml"), BymlFile::TableGeneration::Auto);
	}
}

TemplateMgr::Template* TemplateMgr::GetTemplate(std::string Name)
{
	for (TemplateMgr::Template& Tmpl : TemplateMgr::Templates)
	{
		if (Tmpl.GetName() == Name)
		{
			return &Tmpl;
		}
	}
	return nullptr;
}

std::vector<TemplateMgr::Template>& TemplateMgr::GetTemplates()
{
	return TemplateMgr::Templates;
}

bool TemplateMgr::HasTemplate(std::string Name)
{
	for (TemplateMgr::Template& Tmpl : TemplateMgr::Templates)
	{
		if (Tmpl.GetName() == Name)
		{
			return true;
		}
	}
	return false;
}

void TemplateMgr::AddTemplate(std::string Name, std::vector<Actor> Actors)
{
	Vector3F BaseTranslation = Actors[0].GetTranslate();

	for (Actor& Actor : Actors)
	{
		Actor.GetTranslate().SetX(Actor.GetTranslate().GetX() - BaseTranslation.GetX());
		Actor.GetTranslate().SetY(Actor.GetTranslate().GetY() - BaseTranslation.GetY());
		Actor.GetTranslate().SetZ(Actor.GetTranslate().GetZ() - BaseTranslation.GetZ());
	}
	TemplateMgr::Templates.emplace_back(Name, Actors);
}