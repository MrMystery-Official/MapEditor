#include "MapLoader.h"

void MapLoader::DetectInternalGameVersion()
{
	using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
	for (const auto& dirEntry : recursive_directory_iterator(Config::GetRomFSFile("System/Resource"))) {
		std::string FileName = dirEntry.path().string().substr(dirEntry.path().string().find_last_of("/\\") + 1);
		if (FileName.find("ResourceSizeTable.Product.") != 0 ||
			FileName.find(".rsizetable.zs") == std::string::npos) {
			continue;
		}

		size_t start = FileName.find("ResourceSizeTable.Product.") + strlen("ResourceSizeTable.Product.");
		size_t end = FileName.find(".", start);

		std::string Version = FileName.substr(start, end - start);
		if (stoi(Config::InternalGameVersion) <= stoi(Version)) {
			Config::InternalGameVersion = Version;
		}
	}
}

void InterpretActorNode(Actor* Actor, BymlFile::Node* Node)
{
	Actor->SetGyml(Node->GetChild("Gyaml")->GetValue<std::string>());
	Actor->SetHash(Node->GetChild("Hash")->GetValue<uint64_t>());
	Actor->SetSRTHash(Node->GetChild("SRTHash")->GetValue<uint32_t>());

	if (Actor->GetHash() > HashMgr::CurrentHash) HashMgr::CurrentHash = Actor->GetHash();
	if (Actor->GetSRTHash() > HashMgr::CurrentSRTHash) HashMgr::CurrentSRTHash = Actor->GetSRTHash();

	if (Node->HasChild("Bakeable"))
	{
		Actor->SetBakeable(Node->GetChild("Bakeable")->GetValue<bool>());
	}
	if (Node->HasChild("IsPhysicsStable"))
	{
		Actor->SetPhysicsStable(Node->GetChild("IsPhysicsStable")->GetValue<bool>());
	}
	if (Node->HasChild("MoveRadius"))
	{
		Actor->SetMoveRadius(Node->GetChild("MoveRadius")->GetValue<float>());
	}
	if (Node->HasChild("ExtraCreateRadius"))
	{
		Actor->SetExtraCreateRadius(Node->GetChild("ExtraCreateRadius")->GetValue<float>());
	}
	if (Node->HasChild("IsForceActive"))
	{
		Actor->SetForceActive(Node->GetChild("IsForceActive")->GetValue<bool>());
	}
	if (Node->HasChild("IsInWater"))
	{
		Actor->SetInWater(Node->GetChild("IsInWater")->GetValue<bool>());
	}
	if (Node->HasChild("TurnActorNearEnemy"))
	{
		Actor->SetTurnActorNearEnemy(Node->GetChild("TurnActorNearEnemy")->GetValue<bool>());
	}
	if (Node->HasChild("Name"))
	{
		Actor->SetName(Node->GetChild("Name")->GetValue<std::string>());
	}
	if (Node->HasChild("Translate"))
	{
		Vector3F Translate;
		Translate.SetX(Node->GetChild("Translate")->GetChild(0)->GetValue<float>());
		Translate.SetY(Node->GetChild("Translate")->GetChild(1)->GetValue<float>());
		Translate.SetZ(Node->GetChild("Translate")->GetChild(2)->GetValue<float>());
		Actor->SetTranslate(Translate);
	}
	if (Node->HasChild("Scale"))
	{
		Vector3F Scale;
		Scale.SetX(Node->GetChild("Scale")->GetChild(0)->GetValue<float>());
		Scale.SetY(Node->GetChild("Scale")->GetChild(1)->GetValue<float>());
		Scale.SetZ(Node->GetChild("Scale")->GetChild(2)->GetValue<float>());
		Actor->SetScale(Scale);
	}
	if (Node->HasChild("Rotate"))
	{
		Vector3F Rotate;
		Rotate.SetX(Util::RadianToDegree(Node->GetChild("Rotate")->GetChild(0)->GetValue<float>()));
		Rotate.SetY(Util::RadianToDegree(Node->GetChild("Rotate")->GetChild(1)->GetValue<float>()));
		Rotate.SetZ(Util::RadianToDegree(Node->GetChild("Rotate")->GetChild(2)->GetValue<float>()));
		Actor->SetRotate(Rotate);
	}

	if (Node->HasChild("Phive"))
	{
		BymlFile::Node* PhiveNode = Node->GetChild("Phive");
		if (PhiveNode->HasChild("Rails"))
		{
			for (BymlFile::Node& RailNode : PhiveNode->GetChild("Rails")->GetChildren())
			{
				Actor::Phive::RailData Rail;
				if (RailNode.HasChild("IsClosed"))
				{
					Rail.IsClosed = RailNode.GetChild("IsClosed")->GetValue<bool>();
				}
				if (RailNode.HasChild("Type"))
				{
					Rail.Type = RailNode.GetChild("Type")->GetValue<std::string>();
				}
				if (RailNode.HasChild("Nodes"))
				{
					for (BymlFile::Node& BymlNode : RailNode.GetChild("Nodes")->GetChildren())
					{
						for (BymlFile::Node& SubNode : BymlNode.GetChildren())
						{
							Actor::Phive::RailData::Node RailNode;
							RailNode.Key = SubNode.GetKey();
							std::cout << RailNode.Key << std::endl;

							RailNode.Value.SetX(SubNode.GetChild(0)->GetValue<float>());
							RailNode.Value.SetY(SubNode.GetChild(1)->GetValue<float>());
							RailNode.Value.SetZ(SubNode.GetChild(2)->GetValue<float>());

							Rail.Nodes.push_back(RailNode);
						}
					}
				}

				Actor->GetPhive().Rails.push_back(Rail);
			}
		}
		if (PhiveNode->HasChild("RopeHeadLink"))
		{
			if (PhiveNode->GetChild("RopeHeadLink")->HasChild("ID"))
			{
				Actor->GetPhive().RopeHeadLink.ID = PhiveNode->GetChild("RopeHeadLink")->GetChild("ID")->GetValue<uint64_t>();
			}
			if (PhiveNode->GetChild("RopeHeadLink")->HasChild("Owners"))
			{
				for (BymlFile::Node& OwnerNode : PhiveNode->GetChild("RopeHeadLink")->GetChild("Owners")->GetChildren())
				{
					Actor->GetPhive().RopeHeadLink.Owners.push_back(OwnerNode.GetChild("Refer")->GetValue<uint64_t>());
				}
			}
			if (PhiveNode->GetChild("RopeHeadLink")->HasChild("Refers"))
			{
				for (BymlFile::Node& ReferNode : PhiveNode->GetChild("RopeHeadLink")->GetChild("Refers")->GetChildren())
				{
					Actor->GetPhive().RopeHeadLink.Refers.push_back(ReferNode.GetChild("Owner")->GetValue<uint64_t>());
				}
			}
		}
		if (PhiveNode->HasChild("RopeTailLink"))
		{
			if (PhiveNode->GetChild("RopeTailLink")->HasChild("ID"))
			{
				Actor->GetPhive().RopeTailLink.ID = PhiveNode->GetChild("RopeTailLink")->GetChild("ID")->GetValue<uint64_t>();
			}
			if (PhiveNode->GetChild("RopeTailLink")->HasChild("Owners"))
			{
				for (BymlFile::Node& OwnerNode : PhiveNode->GetChild("RopeTailLink")->GetChild("Owners")->GetChildren())
				{
					Actor->GetPhive().RopeTailLink.Owners.push_back(OwnerNode.GetChild("Refer")->GetValue<uint64_t>());
				}
			}
			if (PhiveNode->GetChild("RopeTailLink")->HasChild("Refers"))
			{
				for (BymlFile::Node& ReferNode : PhiveNode->GetChild("RopeTailLink")->GetChild("Refers")->GetChildren())
				{
					Actor->GetPhive().RopeTailLink.Refers.push_back(ReferNode.GetChild("Owner")->GetValue<uint64_t>());
				}
			}
		}
		if (PhiveNode->HasChild("Placement"))
		{
			for (BymlFile::Node& PlacementNode : PhiveNode->GetChild("Placement")->GetChildren())
			{
				Actor->GetPhive().Placement.insert({ PlacementNode.GetKey(), PlacementNode.GetValue<std::string>() });
			}
		}
		if (PhiveNode->HasChild("ConstraintLink"))
		{
			BymlFile::Node* ConstraintLinkNode = PhiveNode->GetChild("ConstraintLink");
			if (ConstraintLinkNode->HasChild("ID"))
			{
				Actor->GetPhive().ConstraintLink.ID = ConstraintLinkNode->GetChild("ID")->GetValue<uint64_t>();
			}
			if (ConstraintLinkNode->HasChild("Refers"))
			{
				for (BymlFile::Node& ReferNode : ConstraintLinkNode->GetChild("Refers")->GetChildren())
				{
					Actor::Phive::ConstraintLinkData::ReferData Refer;
					if (ReferNode.HasChild("Owner"))
					{
						Refer.Owner = ReferNode.GetChild("Owner")->GetValue<uint64_t>();
					}
					if (ReferNode.HasChild("Type"))
					{
						Refer.Type = ReferNode.GetChild("Type")->GetValue<std::string>();
					}
					Actor->GetPhive().ConstraintLink.Refers.push_back(Refer);
				}
			}
			if (ConstraintLinkNode->HasChild("Owners"))
			{
				for (BymlFile::Node& OwnersNode : ConstraintLinkNode->GetChild("Owners")->GetChildren())
				{
					Actor::Phive::ConstraintLinkData::OwnerData Owner;
					if (OwnersNode.HasChild("BreakableData"))
					{
						for (BymlFile::Node& BreakableDataNode : OwnersNode.GetChild("BreakableData")->GetChildren())
						{
							Owner.BreakableData.insert({ BreakableDataNode.GetKey(), BreakableDataNode.GetValue<std::string>() });
						}
					}
					if (OwnersNode.HasChild("ClusterData"))
					{
						for (BymlFile::Node& ClusterDataNode : OwnersNode.GetChild("ClusterData")->GetChildren())
						{
							Owner.ClusterData.insert({ ClusterDataNode.GetKey(), ClusterDataNode.GetValue<std::string>() });
						}
					}
					if (OwnersNode.HasChild("UserData"))
					{
						for (BymlFile::Node& UserDataNode : OwnersNode.GetChild("UserData")->GetChildren())
						{
							Owner.UserData.insert({ UserDataNode.GetKey(), UserDataNode.GetValue<std::string>() });
						}
					}
					if (OwnersNode.HasChild("OwnerPose"))
					{
						if (OwnersNode.GetChild("OwnerPose")->HasChild("Rotate"))
						{
							Vector3F OwnerPoseRotate;
							OwnerPoseRotate.SetX(Util::RadianToDegree(OwnersNode.GetChild("OwnerPose")->GetChild("Rotate")->GetChild(0)->GetValue<float>()));
							OwnerPoseRotate.SetY(Util::RadianToDegree(OwnersNode.GetChild("OwnerPose")->GetChild("Rotate")->GetChild(1)->GetValue<float>()));
							OwnerPoseRotate.SetZ(Util::RadianToDegree(OwnersNode.GetChild("OwnerPose")->GetChild("Rotate")->GetChild(2)->GetValue<float>()));
							Owner.OwnerPose.Rotate = OwnerPoseRotate;
						}
						if (OwnersNode.GetChild("OwnerPose")->HasChild("Trans"))
						{
							Vector3F OwnerPoseTrans;
							OwnerPoseTrans.SetX(OwnersNode.GetChild("OwnerPose")->GetChild("Trans")->GetChild(0)->GetValue<float>());
							OwnerPoseTrans.SetY(OwnersNode.GetChild("OwnerPose")->GetChild("Trans")->GetChild(1)->GetValue<float>());
							OwnerPoseTrans.SetZ(OwnersNode.GetChild("OwnerPose")->GetChild("Trans")->GetChild(2)->GetValue<float>());
							Owner.OwnerPose.Translate = OwnerPoseTrans;
						}
					}
					if (OwnersNode.HasChild("ParamData"))
					{
						for (BymlFile::Node& ParamDataNode : OwnersNode.GetChild("ParamData")->GetChildren())
						{
							Owner.ParamData.insert({ ParamDataNode.GetKey(), ParamDataNode.GetValue<std::string>() });
						}
					}
					if (OwnersNode.HasChild("PivotData"))
					{
						BymlFile::Node* PivotDataNode = OwnersNode.GetChild("PivotData");
						if (PivotDataNode->HasChild("Axis"))
						{
							Owner.PivotData.Axis = PivotDataNode->GetChild("Axis")->GetValue<int32_t>();
						}
						if (PivotDataNode->HasChild("Pivot"))
						{
							Vector3F Pivot;
							Pivot.SetX(PivotDataNode->GetChild("Pivot")->GetChild(0)->GetValue<float>());
							Pivot.SetY(PivotDataNode->GetChild("Pivot")->GetChild(1)->GetValue<float>());
							Pivot.SetZ(PivotDataNode->GetChild("Pivot")->GetChild(2)->GetValue<float>());
							Owner.PivotData.Pivot = Pivot;
						}
					}
					if (OwnersNode.HasChild("Refer"))
					{
						Owner.Refer = OwnersNode.GetChild("Refer")->GetValue<uint64_t>();
					}
					if (OwnersNode.HasChild("ReferPose"))
					{
						if (OwnersNode.GetChild("ReferPose")->HasChild("Rotate"))
						{
							Vector3F ReferPoseRotate;
							ReferPoseRotate.SetX(Util::RadianToDegree(OwnersNode.GetChild("ReferPose")->GetChild("Rotate")->GetChild(0)->GetValue<float>()));
							ReferPoseRotate.SetY(Util::RadianToDegree(OwnersNode.GetChild("ReferPose")->GetChild("Rotate")->GetChild(1)->GetValue<float>()));
							ReferPoseRotate.SetZ(Util::RadianToDegree(OwnersNode.GetChild("ReferPose")->GetChild("Rotate")->GetChild(2)->GetValue<float>()));
							Owner.ReferPose.Rotate = ReferPoseRotate;
						}
						if (OwnersNode.GetChild("ReferPose")->HasChild("Trans"))
						{
							Vector3F ReferPoseTrans;
							ReferPoseTrans.SetX(OwnersNode.GetChild("ReferPose")->GetChild("Trans")->GetChild(0)->GetValue<float>());
							ReferPoseTrans.SetY(OwnersNode.GetChild("ReferPose")->GetChild("Trans")->GetChild(1)->GetValue<float>());
							ReferPoseTrans.SetZ(OwnersNode.GetChild("ReferPose")->GetChild("Trans")->GetChild(2)->GetValue<float>());
							Owner.ReferPose.Translate = ReferPoseTrans;
						}
					}
					if (OwnersNode.HasChild("Type"))
					{
						Owner.Type = OwnersNode.GetChild("Type")->GetValue<std::string>();
					}
					Actor->GetPhive().ConstraintLink.Owners.push_back(Owner);
				}
			}
		}
	}

	if (Node->HasChild("Rails"))
	{
		BymlFile::Node* RailsNode = Node->GetChild("Rails");
		for (BymlFile::Node& RailsChild : RailsNode->GetChildren())
		{
			Actor::Rail Rail;
			for (BymlFile::Node& RailChild : RailsChild.GetChildren())
			{
				if (RailChild.GetKey() == "Dst")
				{
					Rail.Dst = RailChild.GetValue<uint64_t>();
				}
				else if (RailChild.GetKey() == "Gyaml")
				{
					Rail.Gyaml = RailChild.GetValue<std::string>();
				}
				else if (RailChild.GetKey() == "Name")
				{
					Rail.Name = RailChild.GetValue<std::string>();
				}
			}
			Actor->GetRails().push_back(Rail);
		}
	}

	if (Node->HasChild("Links"))
	{
		BymlFile::Node* LinksNode = Node->GetChild("Links");
		for (BymlFile::Node& LinksChild : LinksNode->GetChildren())
		{
			Actor::Link Link;
			for (BymlFile::Node& LinkChild : LinksChild.GetChildren())
			{
				if (LinkChild.GetKey() == "Dst")
				{
					Link.Dst = LinkChild.GetValue<uint64_t>();
				}
				else if (LinkChild.GetKey() == "Gyaml")
				{
					Link.Gyaml = LinkChild.GetValue<std::string>();
				}
				else if (LinkChild.GetKey() == "Name")
				{
					Link.Name = LinkChild.GetValue<std::string>();
				}
				else if (LinkChild.GetKey() == "Src")
				{
					Link.Src = LinkChild.GetValue<uint64_t>();
				}
			}
			Actor->GetLinks().push_back(Link);
		}
	}

	if (Node->HasChild("Dynamic"))
	{
		for (BymlFile::Node& DynamicNode : Node->GetChild("Dynamic")->GetChildren())
		{
			if (DynamicNode.GetType() != BymlFile::Type::Array)
			{
				Actor->AddDynamic(DynamicNode.GetKey(), DynamicNode.GetValue<std::string>());
			}
			else
			{
				Vector3F VectorValue;
				VectorValue.SetX(DynamicNode.GetChild(0)->GetValue<float>());
				VectorValue.SetY(DynamicNode.GetChild(1)->GetValue<float>());
				VectorValue.SetZ(DynamicNode.GetChild(2)->GetValue<float>());
				Actor->AddDynamic(DynamicNode.GetKey(), VectorValue);
			}
		}
	}

	if (Node->HasChild("Presence"))
	{
		for (BymlFile::Node& PresenceNode : Node->GetChild("Presence")->GetChildren())
		{
			Actor->AddPresence(PresenceNode.GetKey(), PresenceNode.GetValue<std::string>());
		}
	}
}

std::vector<Actor> MapLoader::LoadMap(std::string Key, MapLoader::Type Type, bool LoadMergedActors)
{
	std::vector<Actor> Actors;

	std::string BancPathPrefix = "Banc/";
	switch (Type)
	{
	case MapLoader::Type::Sky:
		BancPathPrefix += "MainField/Sky/";
		break;
	case MapLoader::Type::MainField:
		BancPathPrefix += "MainField/";
		break;
	case MapLoader::Type::MinusField:
		BancPathPrefix += "MinusField/";
		break;
	case MapLoader::Type::SmallDungeon:
		BancPathPrefix += "SmallDungeon/Dungeon";
		break;
	case MapLoader::Type::LargeDungeon:
		BancPathPrefix += "LargeDungeon/LargeDungeon";
		break;
	case MapLoader::Type::NormalStage:
		BancPathPrefix += "NormalStage/";
		break;
	default:
		std::cerr << "Invalid map type! (Don't know how that is even possible)\n";
		return std::vector<Actor>(0);
	}

	Config::BancPrefix = BancPathPrefix;
	if (!Util::FileExists(Config::GetWorkingDirFile("Save/Banc/SmallDungeon/StartPos/SmallDungeon.startpos.byml.zs")))
	{
		Util::CopyFileToDest(Config::GetRomFSFile("Banc/SmallDungeon/StartPos/SmallDungeon.startpos.byml.zs"), Config::GetWorkingDirFile("Save/Banc/SmallDungeon/StartPos/SmallDungeon.startpos.byml.zs"));
	}

	BymlFile StaticActorByml(ZStdFile::Decompress(Config::GetRomFSFile(BancPathPrefix + Key + "_Static.bcett.byml.zs"), ZStdFile::Dictionary::BcettByaml).Data);
	BymlFile DynamicActorByml(ZStdFile::Decompress(Config::GetRomFSFile(BancPathPrefix + Key + "_Dynamic.bcett.byml.zs"), ZStdFile::Dictionary::BcettByaml).Data);

	Config::StaticActorsByml = StaticActorByml;
	Config::DynamicActorsByml = DynamicActorByml;

	Config::Key = Key;
	Config::MapType = (uint8_t)Type;

	std::vector<std::string> DecodedMergedActorFiles;

	int MergedActorIndex = 0;

	for (BymlFile::Node& ActorNode : DynamicActorByml.GetNode("Actors")->GetChildren())
	{
		Actor MapActor;
		MapActor.SetType(Actor::Type::Dynamic);
		InterpretActorNode(&MapActor, &ActorNode);
		Actors.push_back(MapActor);
 		int ActorIndex = Actors.size() - 1;
		if (MapActor.GetDynamic().DynamicString.find("BancPath") != MapActor.GetDynamic().DynamicString.end() && LoadMergedActors)
		{
			if (std::find(DecodedMergedActorFiles.begin(), DecodedMergedActorFiles.end(), MapActor.GetDynamic().DynamicString["BancPath"]) == DecodedMergedActorFiles.end())
			{
				std::cout << "Decoding merged actor " << MapActor.GetDynamic().DynamicString["BancPath"] << std::endl;
				BymlFile MergedActorByml(ZStdFile::Decompress(Config::GetRomFSFile(MapActor.GetDynamic().DynamicString["BancPath"] + ".zs"), ZStdFile::Dictionary::BcettByaml).Data);
				for (BymlFile::Node& ActorNodeMerge : MergedActorByml.GetNode("Actors")->GetChildren())
				{
					Actor ActorMerge;
					ActorMerge.SetType(Actor::Type::Merged);
					InterpretActorNode(&ActorMerge, &ActorNodeMerge);

					ActorMerge.GetTranslate().SetX(ActorMerge.GetTranslate().GetX() + MapActor.GetTranslate().GetX());
					ActorMerge.GetTranslate().SetY(ActorMerge.GetTranslate().GetY() + MapActor.GetTranslate().GetY());
					ActorMerge.GetTranslate().SetZ(ActorMerge.GetTranslate().GetZ() + MapActor.GetTranslate().GetZ());

					ActorMerge.SetMergedActorIndex(MergedActorIndex);
					ActorMerge.SetMergedActorParentIndex(ActorIndex);

					Actors.push_back(ActorMerge);
				}
				DecodedMergedActorFiles.push_back(MapActor.GetDynamic().DynamicString["BancPath"]);

				Config::MergedActorBymls.insert({ ActorIndex, MergedActorByml });

				MergedActorIndex++;
			}
			else
			{
				std::cout << "Already decoded merged actor file " << MapActor.GetDynamic().DynamicString["BancPath"] << std::endl;
			}
		}
	}

	for (BymlFile::Node& ActorNode : StaticActorByml.GetNode("Actors")->GetChildren())
	{
		Actor MapActor;
		MapActor.SetType(Actor::Type::Static);
		InterpretActorNode(&MapActor, &ActorNode);
		Actors.push_back(MapActor);
		int ActorIndex = Actors.size() - 1;
		if (MapActor.GetDynamic().DynamicString.find("BancPath") != MapActor.GetDynamic().DynamicString.end() && LoadMergedActors)
		{
			if (std::find(DecodedMergedActorFiles.begin(), DecodedMergedActorFiles.end(), MapActor.GetDynamic().DynamicString["BancPath"]) == DecodedMergedActorFiles.end())
			{
				std::cout << "Decoding merged actor " << MapActor.GetDynamic().DynamicString["BancPath"] << std::endl;
				BymlFile MergedActorByml(ZStdFile::Decompress(Config::GetRomFSFile(MapActor.GetDynamic().DynamicString["BancPath"] + ".zs"), ZStdFile::Dictionary::BcettByaml).Data);
				for (BymlFile::Node& ActorNodeMerge : MergedActorByml.GetNode("Actors")->GetChildren())
				{
					Actor ActorMerge;
					ActorMerge.SetType(Actor::Type::Merged);
					InterpretActorNode(&ActorMerge, &ActorNodeMerge);

					ActorMerge.GetTranslate().SetX(ActorMerge.GetTranslate().GetX() + MapActor.GetTranslate().GetX());
					ActorMerge.GetTranslate().SetY(ActorMerge.GetTranslate().GetY() + MapActor.GetTranslate().GetY());
					ActorMerge.GetTranslate().SetZ(ActorMerge.GetTranslate().GetZ() + MapActor.GetTranslate().GetZ());

					ActorMerge.SetMergedActorIndex(MergedActorIndex);
					ActorMerge.SetMergedActorParentIndex(ActorIndex);

					Actors.push_back(ActorMerge);
				}
				DecodedMergedActorFiles.push_back(MapActor.GetDynamic().DynamicString["BancPath"]);

				Config::MergedActorBymls.insert({ ActorIndex, MergedActorByml });

				MergedActorIndex++;
			}
			else
			{
				std::cout << "Already decoded merged actor file " << MapActor.GetDynamic().DynamicString["BancPath"] << std::endl;
			}
		}
	}

	std::map<std::string, std::string> ActorModelNames;
	std::map<std::string, std::string> ActorCategories;

	/* Actor model loading */
	for (Actor& Actor : Actors)
	{
		if (Actor.GetGyml().find("Area") != std::string::npos)
		{
			if (Actor.GetGyml().find("Forbid") == std::string::npos)
			{
				Actor.SetModel(ActorModelLibrary::GetModel("Area"));
			}
			else
			{
				Actor.SetModel(ActorModelLibrary::GetModel("ForbidArea"));
			}
			Actor.SetCategory("System");
			continue;
		}

		if (Actor.GetGyml().rfind("MapEditor_Collision_", 0) == 0)
		{
			Actor.SetModel(ActorModelLibrary::GetModel("Collision"));
			Actor.SetCategory("Editor");
			continue;
		}

		if (!ActorModelNames.count(Actor.GetGyml()))
		{
			ZStdFile::Result Result = ZStdFile::Decompress(Config::GetRomFSFile("Pack/Actor/" + Actor.GetGyml() + ".pack.zs"), ZStdFile::Dictionary::Pack);

			if (Result.Data.size() == 0)
			{
				ActorModelNames[Actor.GetGyml()] = "None";
				ActorCategories[Actor.GetGyml()] = "Editor";
				Actor.SetModel(ActorModelLibrary::GetModel("None"));
				continue;
			}

			SarcFile ActorPackFile(Result.Data);

			std::string ModelInfoEntryName;
			for (SarcFile::Entry Entry : ActorPackFile.GetEntries())
			{
				if (Entry.Name.rfind("Component/ModelInfo/", 0) == 0)
				{
					if (ModelInfoEntryName.rfind("Component/ModelInfo/None", 0) == 0)
					{
						ModelInfoEntryName = Entry.Name;
						break;
					}
					else
					{
						/* Parse model */
						BymlFile ModelInfo(ActorPackFile.GetEntry(Entry.Name).Bytes);
						if (ModelInfo.HasChild("ModelProjectName") && ModelInfo.HasChild("FmdbName"))
						{
							ModelInfoEntryName = Entry.Name;
							break;
						}
					}
				}
			}

			/* Parse Category */
			BymlFile ActorSystemSettings(ActorPackFile.GetEntry("Actor/" + Actor.GetGyml() + ".engine__actor__ActorParam.bgyml").Bytes);
			if (ActorSystemSettings.HasChild("Category"))
			{
				Actor.SetCategory(ActorSystemSettings.GetNode("Category")->GetValue<std::string>());
			}
			else
			{
				std::string ParentPath = ActorSystemSettings.GetNode("$parent")->GetValue<std::string>();
				ParentPath = ParentPath.substr(5, ParentPath.length() - 9) + "bgyml";
				BymlFile Parent(ActorPackFile.GetEntry(ParentPath).Bytes);
				if (Parent.HasChild("Category"))
				{
					Actor.SetCategory("Unknown");
				}
			}

			ActorCategories[Actor.GetGyml()] = Actor.GetCategory();

			if (ModelInfoEntryName.rfind("Component/ModelInfo/None", 0) == 0 || ModelInfoEntryName == "")
			{
				ActorModelNames[Actor.GetGyml()] = "None";
				Actor.SetModel(ActorModelLibrary::GetModel("None"));
				std::cout << "Warning: Could not find a model for actor " << Actor.GetGyml() << "!\n";
				continue;
			}

			BymlFile ModelInfo(ActorPackFile.GetEntry(ModelInfoEntryName).Bytes);

			if (ModelInfo.GetNodes().size() == 0)
			{
				Actor.SetModel(ActorModelLibrary::GetModel("None"));
				continue;
			}
			std::cout << Actor.GetGyml() << std::endl;
			Actor.SetModel(ActorModelLibrary::GetModel(ModelInfo.GetNode("ModelProjectName")->GetValue<std::string>() + "." + ModelInfo.GetNode("FmdbName")->GetValue<std::string>()));
			ActorModelNames[Actor.GetGyml()] = ModelInfo.GetNode("ModelProjectName")->GetValue<std::string>() + "." + ModelInfo.GetNode("FmdbName")->GetValue<std::string>();
		}
		else
		{
			Actor.SetModel(ActorModelLibrary::GetModel(ActorModelNames[Actor.GetGyml()]));
			Actor.SetCategory(ActorCategories[Actor.GetGyml()]);
		}
	}

	std::map<std::string, uint32_t> ActorCount;

	for (Actor MapActor : Actors)
	{
		if (ActorCount.find(MapActor.GetGyml()) != ActorCount.end())
		{
			ActorCount[MapActor.GetGyml()] += 1;
		}
		else
		{
			ActorCount.insert({ MapActor.GetGyml(), 1 });
		}
	}

	for (auto& [Key, Val] : ActorCount)
	{
		//std::cout << Key << ": " << Val << std::endl;
	}

	MapConfig::Load(&Actors);

	return Actors;
}