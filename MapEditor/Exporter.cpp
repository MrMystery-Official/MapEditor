#include "Exporter.h"
#include "MapLoader.h"

void WriteBymlVector(BymlFile::Node* Root, std::string Key, Vector3F Vector, Vector3F DefaultValues, bool SkipDefaultVectorCheck)
{
	if (Vector.GetX() != DefaultValues.GetX() || Vector.GetY() != DefaultValues.GetY() || Vector.GetZ() != DefaultValues.GetZ() || SkipDefaultVectorCheck)
	{
		BymlFile::Node VectorRoot(BymlFile::Type::Array, Key);

		BymlFile::Node VectorX(BymlFile::Type::Float, "0");
		VectorX.SetValue<float>(Vector.GetX());
		VectorRoot.AddChild(VectorX);

		BymlFile::Node VectorY(BymlFile::Type::Float, "1");
		VectorY.SetValue<float>(Vector.GetY());
		VectorRoot.AddChild(VectorY);

		BymlFile::Node VectorZ(BymlFile::Type::Float, "2");
		VectorZ.SetValue<float>(Vector.GetZ());
		VectorRoot.AddChild(VectorZ);

		Root->AddChild(VectorRoot);
	}
}

BymlFile::Type GetDynamicValueDataType(std::string Value)
{
	if (Value.empty())
	{
		return BymlFile::Type::StringIndex;
	}

	if (Value == "true" || Value == "false")
	{
		return BymlFile::Type::Bool;
	}

	// Value for signed integers
	if (Value.find_first_not_of("0123456789") == Value.npos && Value.find("-") != std::string::npos)
	{
		if (Value.size() <= 11)
		{
			return BymlFile::Type::Int32;
		}
		else if (Value.size() <= 20)
		{
			return BymlFile::Type::Int64;
		}
		else
		{
			return BymlFile::Type::Null;
		}
	}

	// Check for unsigned integers
	if (Value.find_first_not_of("0123456789") == Value.npos)
	{
		if (Value.size() <= 10)
		{
			return BymlFile::Type::UInt32;
		}
		else if (Value.size() <= 20) 
		{
			return BymlFile::Type::UInt64;
		}
		else 
		{
			return BymlFile::Type::Null;
		}
	}

	// Check for floats
	if (Value.find(".") != std::string::npos && Value.find_first_not_of("0123456789.") == Value.npos) 
	{
		return BymlFile::Type::Float;
	}

	// Default to String
	return BymlFile::Type::StringIndex;
}

void ReplaceStringInBymlNodes(BymlFile::Node* Node, std::string Search, std::string Replace) {
	Util::ReplaceString(Node->GetKey(), Search, Replace);
	if (Node->GetType() == BymlFile::Type::StringIndex)
	{
		std::string Value = Node->GetValue<std::string>();
		Util::ReplaceString(Value, Search, Replace);
		Node->SetValue<std::string>(Value);
	}

	for (BymlFile::Node& Child : Node->GetChildren()) {
		ReplaceStringInBymlNodes(&Child, Search, Replace);
	}
}

void CreateExportOnlyFiles(std::vector<Actor>* Actors, std::string Path)
{
	if (Util::FileExists(Config::GetWorkingDirFile("Save/Pack/Actor")))
	{
		std::vector<std::string> CollisionActors;

		for (const auto& Entry : std::filesystem::directory_iterator(Config::GetWorkingDirFile("Save/Pack/Actor")))
		{
			if (Entry.is_regular_file() && Entry.path().filename().string().rfind("MapEditor_Collision_", 0) == 0)
			{
				CollisionActors.push_back(Entry.path().filename().string().substr(0, Entry.path().filename().string().size() - 8)); // 8 = .pack.zs
			}
		}

		/* ActorInfo */
		if (!CollisionActors.empty())
		{
			BymlFile ActorInfoByml(ZStdFile::Decompress(Config::GetRomFSFile("RSDB/ActorInfo.Product." + Config::GetInternalGameVersion() + ".rstbl.byml.zs"), ZStdFile::Dictionary::Base).Data);

			//This loop sees if a new actor was added since the last time the RSDB was generated -> this saves a HUGE amount of time every export
			for (BymlFile::Node& Node : ActorInfoByml.GetNodes())
			{
				if (Node.HasChild("__RowId"))
				{
					for (std::string CActor : CollisionActors)
					{
						if (Node.GetChild("__RowId")->GetValue<std::string>() == CActor)
						{
							goto RESTBLGeneration;
						}
					}
				}
			}

			ActorInfoByml.NodeCaching = true;

			for (int i = 0; i < CollisionActors.size(); i++)
			{
				BymlFile::Node NewActorDict(BymlFile::Type::Dictionary, std::to_string(ActorInfoByml.GetNodes().size()));

				BymlFile::Node NewActorCalcRadius(BymlFile::Type::Float, "CalcRadius");
				NewActorCalcRadius.SetValue<float>(110.0f);

				BymlFile::Node NewActorCategory(BymlFile::Type::StringIndex, "Category");
				NewActorCategory.SetValue<std::string>("System");

				BymlFile::Node NewActorClassName(BymlFile::Type::StringIndex, "ClassName");
				NewActorClassName.SetValue<std::string>("GameActor");

				BymlFile::Node NewActorCreateRadiusOffset(BymlFile::Type::Float, "CreateRadiusOffset");
				NewActorCreateRadiusOffset.SetValue<float>(5.0f);

				BymlFile::Node NewActorDeleteRadiusOffset(BymlFile::Type::Float, "DeleteRadiusOffset");
				NewActorDeleteRadiusOffset.SetValue<float>(10.0f);

				BymlFile::Node NewActorDisplayRadius(BymlFile::Type::Float, "DisplayRadius");
				NewActorDisplayRadius.SetValue<float>(110.0f);

				BymlFile::Node NewActorELinkUserName(BymlFile::Type::StringIndex, "ELinkUserName");
				NewActorELinkUserName.SetValue<std::string>(CollisionActors[i]);

				BymlFile::Node NewActorInstanceHeapSize(BymlFile::Type::Int32, "InstanceHeapSize");
				NewActorInstanceHeapSize.SetValue<int32_t>(77776);

				BymlFile::Node NewActorLoadRadius(BymlFile::Type::Float, "LoadRadius");
				NewActorLoadRadius.SetValue<float>(210.0f);

				BymlFile::Node NewActorModelAabbMax(BymlFile::Type::Dictionary, "ModelAabbMax");
				BymlFile::Node NewActorModelAabbMaxX(BymlFile::Type::Float, "X");
				NewActorModelAabbMaxX.SetValue<float>(1.0f);
				BymlFile::Node NewActorModelAabbMaxY(BymlFile::Type::Float, "Y");
				NewActorModelAabbMaxY.SetValue<float>(2.0f);
				BymlFile::Node NewActorModelAabbMaxZ(BymlFile::Type::Float, "Z");
				NewActorModelAabbMaxZ.SetValue<float>(4.0f);
				NewActorModelAabbMax.AddChild(NewActorModelAabbMaxX);
				NewActorModelAabbMax.AddChild(NewActorModelAabbMaxY);
				NewActorModelAabbMax.AddChild(NewActorModelAabbMaxZ);

				BymlFile::Node NewActorModelAabbMin(BymlFile::Type::Dictionary, "ModelAabbMin");
				BymlFile::Node NewActorModelAabbMinX(BymlFile::Type::Float, "X");
				NewActorModelAabbMinX.SetValue<float>(-1.0f);
				BymlFile::Node NewActorModelAabbMinY(BymlFile::Type::Float, "Y");
				NewActorModelAabbMinY.SetValue<float>(0.0f);
				BymlFile::Node NewActorModelAabbMinZ(BymlFile::Type::Float, "Z");
				NewActorModelAabbMinZ.SetValue<float>(-4.0f);
				NewActorModelAabbMin.AddChild(NewActorModelAabbMinX);
				NewActorModelAabbMin.AddChild(NewActorModelAabbMinY);
				NewActorModelAabbMin.AddChild(NewActorModelAabbMinZ);

				BymlFile::Node NewActorPreActorRenderInfoHeapSize(BymlFile::Type::Int32, "PreActorRenderInfoHeapSize");
				NewActorPreActorRenderInfoHeapSize.SetValue<int32_t>(13200);

				BymlFile::Node NewActorUnloadRadiusOffset(BymlFile::Type::Float, "UnloadRadiusOffset");
				NewActorUnloadRadiusOffset.SetValue<float>(10.0f);

				BymlFile::Node NewActorRowId(BymlFile::Type::StringIndex, "__RowId");
				NewActorRowId.SetValue<std::string>(CollisionActors[i]);

				NewActorDict.AddChild(NewActorCalcRadius);
				NewActorDict.AddChild(NewActorCategory);
				NewActorDict.AddChild(NewActorClassName);
				NewActorDict.AddChild(NewActorCreateRadiusOffset);
				NewActorDict.AddChild(NewActorDeleteRadiusOffset);
				NewActorDict.AddChild(NewActorDisplayRadius);
				NewActorDict.AddChild(NewActorELinkUserName);
				NewActorDict.AddChild(NewActorInstanceHeapSize);
				NewActorDict.AddChild(NewActorLoadRadius);
				NewActorDict.AddChild(NewActorModelAabbMax);
				NewActorDict.AddChild(NewActorModelAabbMin);
				NewActorDict.AddChild(NewActorPreActorRenderInfoHeapSize);
				NewActorDict.AddChild(NewActorUnloadRadiusOffset);
				NewActorDict.AddChild(NewActorRowId);

				ActorInfoByml.GetNodes().push_back(NewActorDict);

				ActorInfoByml.AddStringTableEntry(CollisionActors[i]);
			}
			Util::CreateDir(Path + "/RSDB");
			ZStdFile::Compress(ActorInfoByml.ToBinary(BymlFile::TableGeneration::Manual), ZStdFile::Dictionary::Base).WriteToFile(Config::GetWorkingDirFile("Save/RSDB/ActorInfo.Product." + Config::GetInternalGameVersion() + ".rstbl.byml.zs"));
		}
	}

RESTBLGeneration:

	/* RESTBL */
	{
		ResTableFile ResTable(ZStdFile::Decompress(Config::GetRomFSFile("System/Resource/ResourceSizeTable.Product." + Config::GetInternalGameVersion() + ".rsizetable.zs"), ZStdFile::Dictionary::None).Data);

		for (const auto& Entry : std::filesystem::directory_iterator(Config::GetWorkingDirFile("Save")))
		{
			if (Entry.is_regular_file() && Entry.path().filename().string().rfind("Map_", 0) == 0 && Util::EndsWith(Entry.path().filename().string(), ".conf"))
			{
				int MapType = Entry.path().filename().string()[4] - 48; //-48 to go from char to int
				std::string Identifier = Entry.path().filename().string().substr(6, 3);
				std::cout << "MapType: " << MapType << ", Identifier: " << Identifier << std::endl;
				if (MapType == 0) //Sky
				{
					Identifier = Entry.path().filename().string();
					Util::ReplaceString(Identifier, "Map_0_", "");
					Identifier = Identifier.substr(0, Identifier.size() - 5);
					ResTable.SetFileSize("Banc/MainField/Sky/" + Identifier + "_Static.bcett.byml", (ZStdFile::GetDecompressedFileSize(Config::GetWorkingDirFile("Save/Banc/MainField/Sky/" + Identifier + "_Static.bcett.byml.zs"), ZStdFile::Dictionary::BcettByaml) + 1000) * 8);
					ResTable.SetFileSize("Banc/MainField/Sky/" + Identifier + "_Dynamic.bcett.byml", (ZStdFile::GetDecompressedFileSize(Config::GetWorkingDirFile("Save/Banc/MainField/Sky/" + Identifier + "_Dynamic.bcett.byml.zs"), ZStdFile::Dictionary::BcettByaml) + 1000) * 8);
				}
				else if (MapType == 1) //MainField
				{
					ResTable.SetFileSize("Banc/MainField/" + Identifier + "_Static.bcett.byml", (ZStdFile::GetDecompressedFileSize(Config::GetWorkingDirFile("Save/Banc/MainField/" + Identifier + "_Static.bcett.byml.zs"), ZStdFile::Dictionary::BcettByaml) + 1000) * 8);
					ResTable.SetFileSize("Banc/MainField/" + Identifier + "_Dynamic.bcett.byml", (ZStdFile::GetDecompressedFileSize(Config::GetWorkingDirFile("Save/Banc/MainField/" + Identifier + "_Dynamic.bcett.byml.zs"), ZStdFile::Dictionary::BcettByaml) + 1000) * 8);
				}
				else if (MapType == 2) //MinusField
				{
					ResTable.SetFileSize("Banc/MinusField/" + Identifier + "_Static.bcett.byml", (ZStdFile::GetDecompressedFileSize(Config::GetWorkingDirFile("Save/Banc/MinusField/" + Identifier + "_Static.bcett.byml.zs"), ZStdFile::Dictionary::BcettByaml) + 1000) * 8);
					ResTable.SetFileSize("Banc/MinusField/" + Identifier + "_Dynamic.bcett.byml", (ZStdFile::GetDecompressedFileSize(Config::GetWorkingDirFile("Save/Banc/MinusField/" + Identifier + "_Dynamic.bcett.byml.zs"), ZStdFile::Dictionary::BcettByaml) + 1000) * 8);
				}
				else if (MapType == 3) //SmallDungeon
				{
					ResTable.SetFileSize("Banc/SmallDungeon/Dungeon" + Identifier + "_Static.bcett.byml", (ZStdFile::GetDecompressedFileSize(Config::GetWorkingDirFile("Save/Banc/SmallDungeon/Dungeon" + Identifier + "_Static.bcett.byml.zs"), ZStdFile::Dictionary::BcettByaml) + 1000) * 8);
					ResTable.SetFileSize("Banc/SmallDungeon/Dungeon" + Identifier + "_Dynamic.bcett.byml", (ZStdFile::GetDecompressedFileSize(Config::GetWorkingDirFile("Save/Banc/SmallDungeon/Dungeon" + Identifier + "_Dynamic.bcett.byml.zs"), ZStdFile::Dictionary::BcettByaml) + 1000) * 8);
				}
				else if (MapType == 3) //LargeDungeon
				{
					ResTable.SetFileSize("Banc/LargeDungeon/LargeDungeon" + Identifier + "_Static.bcett.byml", (ZStdFile::GetDecompressedFileSize(Config::GetWorkingDirFile("Save/Banc/LargeDungeon/LargeDungeon" + Identifier + "_Static.bcett.byml.zs"), ZStdFile::Dictionary::BcettByaml) + 1000) * 8);
					ResTable.SetFileSize("Banc/LargeDungeon/LargeDungeon" + Identifier + "_Dynamic.bcett.byml", (ZStdFile::GetDecompressedFileSize(Config::GetWorkingDirFile("Save/Banc/LargeDungeon/LargeDungeon" + Identifier + "_Dynamic.bcett.byml.zs"), ZStdFile::Dictionary::BcettByaml) + 1000) * 8);
				}
				else if (MapType == 5) //NormalStage
				{
					ResTable.SetFileSize("Banc/NormalStage/" + Identifier + "_Static.bcett.byml", (ZStdFile::GetDecompressedFileSize(Config::GetWorkingDirFile("Save/Banc/NormalStage/" + Identifier + "_Static.bcett.byml.zs"), ZStdFile::Dictionary::BcettByaml) + 1000) * 8);
					ResTable.SetFileSize("Banc/NormalStage/" + Identifier + "_Dynamic.bcett.byml", (ZStdFile::GetDecompressedFileSize(Config::GetWorkingDirFile("Save/Banc/NormalStage/" + Identifier + "_Dynamic.bcett.byml.zs"), ZStdFile::Dictionary::BcettByaml) + 1000) * 8);
				}
			}
		}
		for (const auto& Entry : std::filesystem::directory_iterator(Config::GetWorkingDirFile("Save/Logic")))
		{
			if (Entry.is_regular_file())
			{
				std::ifstream File(Entry.path(), std::ifstream::ate | std::ifstream::binary);
				ResTable.SetFileSize("Logic/" + Entry.path().filename().string(), File.tellg() * 2);
				File.close();
			}
		}

		auto EstimateMergedDirectory = [](std::string Path, ResTableFile& Table) {
			for (const auto& Entry : std::filesystem::directory_iterator(Config::GetWorkingDirFile("Save/" + Path)))
			{
				if (Entry.is_regular_file() && Util::EndsWith(Entry.path().filename().string(), ".bcett.byml.zs"))
				{
					Table.SetFileSize(Path + "/" + Entry.path().filename().string().substr(0, Entry.path().filename().string().length() - 3), (ZStdFile::GetDecompressedFileSize(Entry.path().string(), ZStdFile::Dictionary::BcettByaml) + 1000) * 8);
				}
			}
		};

		EstimateMergedDirectory("Banc/SmallDungeon/Merged", ResTable);
		EstimateMergedDirectory("Banc/LargeDungeon/Merged", ResTable);
		EstimateMergedDirectory("Banc/MainField/Merged", ResTable);
		EstimateMergedDirectory("Banc/MinusField/Merged", ResTable);
		EstimateMergedDirectory("Banc/NormalStage/Merged", ResTable);

		Util::CreateDir(Path + "/System");
		Util::CreateDir(Path + "/System/Resource");
		ZStdFile::Compress(ResTable.ToBinary(), ZStdFile::Dictionary::None).WriteToFile(Path + "/System/Resource/ResourceSizeTable.Product." + Config::GetInternalGameVersion() + ".rsizetable.zs");
	}
}

BymlFile::Node Exporter::ActorToByml(std::string Path, bool CreateCollisionActors, std::vector<Actor>* Actors, Actor& ExportedActor)
{
	if (ExportedActor.GetGyml().rfind("MapEditor_Collision_", 0) == 0 && CreateCollisionActors)
	{
		ExportedActor.SetType(Actor::Type::Static);
		if (ExportedActor.GetGyml().rfind("MapEditor_Collision_Cube_", 0) == 0)
		{
			BymlFile ShapeParamByml;
			ShapeParamByml.GetType() = BymlFile::Type::Dictionary;
			BymlFile::Node BoxArrayNode(BymlFile::Type::Array, "Box");
			BymlFile::Node BoxShapeDict(BymlFile::Type::Dictionary);

			BymlFile::Node BoxCenterDict(BymlFile::Type::Dictionary, "Center");
			BymlFile::Node BoxCenterX(BymlFile::Type::Float, "X");
			BoxCenterX.SetValue<float>(0);
			BymlFile::Node BoxCenterY(BymlFile::Type::Float, "Y");
			BoxCenterY.SetValue<float>(0);
			BymlFile::Node BoxCenterZ(BymlFile::Type::Float, "Z");
			BoxCenterZ.SetValue<float>(0);
			BoxCenterDict.AddChild(BoxCenterX);
			BoxCenterDict.AddChild(BoxCenterY);
			BoxCenterDict.AddChild(BoxCenterZ);

			BymlFile::Node BoxHalfExtentsDict(BymlFile::Type::Dictionary, "HalfExtents");
			BymlFile::Node BoxHalfExtentsX(BymlFile::Type::Float, "X");
			BoxHalfExtentsX.SetValue<float>(0.5f);
			BymlFile::Node BoxHalfExtentsY(BymlFile::Type::Float, "Y");
			BoxHalfExtentsY.SetValue<float>(0.5f);
			BymlFile::Node BoxHalfExtentsZ(BymlFile::Type::Float, "Z");
			BoxHalfExtentsZ.SetValue<float>(0.5f);
			BoxHalfExtentsDict.AddChild(BoxHalfExtentsX);
			BoxHalfExtentsDict.AddChild(BoxHalfExtentsY);
			BoxHalfExtentsDict.AddChild(BoxHalfExtentsZ);

			BymlFile::Node BoxMaterialsDict(BymlFile::Type::Array, "MaterialPresets");
			if (!ExportedActor.IsCollisionClimbable())
			{
				BymlFile::Node MaterialNode(BymlFile::Type::StringIndex, "Wall_NoClimb");
				MaterialNode.SetValue<std::string>("Wall_NoClimb");
				BoxMaterialsDict.AddChild(MaterialNode);
			}

			BoxShapeDict.AddChild(BoxCenterDict);
			BoxShapeDict.AddChild(BoxHalfExtentsDict);
			if (!BoxMaterialsDict.GetChildren().empty())
			{
				BoxShapeDict.AddChild(BoxMaterialsDict);
			}

			BoxArrayNode.AddChild(BoxShapeDict);

			ShapeParamByml.GetNodes().push_back(BoxArrayNode);

			SarcFile ActorPackSarc("MapEditor_Bake_Collision.pack");

			ActorPackSarc.GetEntry("Phive/ShapeParam/MapEditor_Bake_Collision__Physical.phive__ShapeParam.bgyml").Bytes = ShapeParamByml.ToBinary(BymlFile::TableGeneration::Auto);

			std::vector<std::string> BymlsToReplaceData = { "Component/ELink/MapEditor_Bake_Collision.engine__component__ELinkParam.bgyml", "Component/Physics/MapEditor_Bake_Collision.engine__component__PhysicsParam.bgyml", "Actor/MapEditor_Bake_Collision.engine__actor__ActorParam.bgyml", "Phive/ControllerSetParam/MapEditor_Bake_Collision.phive__ControllerSetParam.bgyml", "Phive/ShapeParam/MapEditor_Bake_Collision__Physical.phive__ShapeParam.bgyml" };
			for (int i = 0; i < BymlsToReplaceData.size(); i++) {
				BymlFile File(ActorPackSarc.GetEntry(BymlsToReplaceData[i]).Bytes);
				for (BymlFile::Node& Nodes : File.GetNodes())
				{
					ReplaceStringInBymlNodes(&Nodes, "MapEditor_Bake_Collision", ExportedActor.GetGyml());
				}
				ActorPackSarc.GetEntry(BymlsToReplaceData[i]).Bytes = File.ToBinary(BymlFile::TableGeneration::Auto);
			}



			for (SarcFile::Entry& Value : ActorPackSarc.GetEntries())
			{
				Util::ReplaceString(Value.Name, "MapEditor_Bake_Collision", ExportedActor.GetGyml());
			}


			//ActorPackSarc.WriteToFile(Config::GetWorkingDirFile("Cache/MapEditor_Bake_Collision_Cube_" + std::to_string(ExportedExportedActor.GetSRTHash()) + ".pack"));
			//ZStdFile ActorPackSarcCompressed(Util::GetWorkingDirFile("Cache/MapEditor_Bake_Collision_RCube_" + std::to_string(actor.SRTHash) + ".pack"), ZStdMode::ZStdDictionary::PackZSDictionary, ZStdMode::ZStdOperation::Compress);
			Util::CreateDir(Path + "/Pack");
			Util::CreateDir(Path + "/Pack/Actor");
			//ActorPackSarcCompressed.Compress(ExportPath + "/Pack/Actor/MapEditor_Bake_Collision_RCube_" + std::to_string(actor.SRTHash) + ".pack.zs");
			ZStdFile::Compress(ActorPackSarc.ToBinary(), ZStdFile::Dictionary::Pack).WriteToFile(Path + "/Pack/Actor/" + ExportedActor.GetGyml() + ".pack.zs");
		}

		if (ExportedActor.GetGyml().rfind("MapEditor_Collision_File_", 0) == 0)
		{
			BymlFile ShapeParamByml;
			ShapeParamByml.GetType() = BymlFile::Type::Dictionary;
			BymlFile::Node PhiveShapeNode(BymlFile::Type::Array, "PhshMesh");
			BymlFile::Node PhiveShapeDict(BymlFile::Type::Dictionary);
			BymlFile::Node PhiveShapeValue(BymlFile::Type::StringIndex, "PhshMeshPath");
			PhiveShapeValue.SetValue<std::string>("Work/Phive/Shape/Dcc/" + ExportedActor.GetCollisionFile());

			PhiveShapeDict.AddChild(PhiveShapeValue);
			PhiveShapeNode.AddChild(PhiveShapeDict);
			ShapeParamByml.GetNodes().push_back(PhiveShapeNode);

			SarcFile ActorPackSarc("MapEditor_Bake_Collision.pack");

			ActorPackSarc.GetEntry("Phive/ShapeParam/MapEditor_Bake_Collision__Physical.phive__ShapeParam.bgyml").Bytes = ShapeParamByml.ToBinary(BymlFile::TableGeneration::Auto);

			std::vector<std::string> BymlsToReplaceData = { "Component/ELink/MapEditor_Bake_Collision.engine__component__ELinkParam.bgyml", "Component/Physics/MapEditor_Bake_Collision.engine__component__PhysicsParam.bgyml", "Actor/MapEditor_Bake_Collision.engine__actor__ActorParam.bgyml", "Phive/ControllerSetParam/MapEditor_Bake_Collision.phive__ControllerSetParam.bgyml", "Phive/ShapeParam/MapEditor_Bake_Collision__Physical.phive__ShapeParam.bgyml" };
			for (int i = 0; i < BymlsToReplaceData.size(); i++) {
				BymlFile File(ActorPackSarc.GetEntry(BymlsToReplaceData[i]).Bytes);
				for (BymlFile::Node& Nodes : File.GetNodes())
				{
					ReplaceStringInBymlNodes(&Nodes, "MapEditor_Bake_Collision", ExportedActor.GetGyml());
				}
				ActorPackSarc.GetEntry(BymlsToReplaceData[i]).Bytes = File.ToBinary(BymlFile::TableGeneration::Auto);
			}

			for (SarcFile::Entry& Value : ActorPackSarc.GetEntries())
			{
				Util::ReplaceString(Value.Name, "MapEditor_Bake_Collision", ExportedActor.GetGyml());
			}


			//ActorPackSarc.WriteToFile(Config::GetWorkingDirFile("Cache/MapEditor_Bake_Collision_Cube_" + std::to_string(ExportedExportedActor.GetSRTHash()) + ".pack"));
			//ZStdFile ActorPackSarcCompressed(Util::GetWorkingDirFile("Cache/MapEditor_Bake_Collision_RCube_" + std::to_string(actor.SRTHash) + ".pack"), ZStdMode::ZStdDictionary::PackZSDictionary, ZStdMode::ZStdOperation::Compress);
			Util::CreateDir(Path + "/Pack");
			Util::CreateDir(Path + "/Pack/Actor");
			//ActorPackSarcCompressed.Compress(ExportPath + "/Pack/Actor/MapEditor_Bake_Collision_RCube_" + std::to_string(actor.SRTHash) + ".pack.zs");
			ZStdFile::Compress(ActorPackSarc.ToBinary(), ZStdFile::Dictionary::Pack).WriteToFile(Path + "/Pack/Actor/" + ExportedActor.GetGyml() + ".pack.zs");
		}

		if (ExportedActor.GetGyml().rfind("MapEditor_Collision_Custom_", 0) == 0)
		{
			BymlFile ShapeParamByml;
			ShapeParamByml.GetType() = BymlFile::Type::Dictionary;
			BymlFile::Node PolytopeArrayNode(BymlFile::Type::Array, "Polytope");
			BymlFile::Node PolytopeShapeDict(BymlFile::Type::Dictionary);

			BymlFile::Node PolytopeMaterialsDict(BymlFile::Type::Array, "MaterialPresets");
			{
				BymlFile::Node PolytopeMaterial(BymlFile::Type::StringIndex, "Material_Stone");
				PolytopeMaterial.SetValue<std::string>("Material_Stone");
				PolytopeMaterialsDict.AddChild(PolytopeMaterial);
			}
			PolytopeShapeDict.AddChild(PolytopeMaterialsDict);

			BymlFile::Node PolytopeVertices(BymlFile::Type::Array, "Vertices");

			BfresFile* BfresModel = nullptr;

			for (auto& LinkedActor : *Actors) {
				if (LinkedActor.GetSRTHash() == ExportedActor.GetCollisionSRTHash()) {
					BfresModel = LinkedActor.GetModel();
					break;
				}
			}

			if (BfresModel == nullptr)
			{
				std::cerr << "ERROR: Could not find custom collision actor parent!\n";
				return BymlFile::Node(BymlFile::Type::Null);
			}
			else
			{
				for (int i = 0; i < BfresModel->GetModels()[0].Vertices.size(); i++)
				{
					for (int VerticeIndex = 0; VerticeIndex < BfresModel->GetModels()[0].Vertices[i].size() / 3; VerticeIndex++)
					{
						BymlFile::Node PolytopeVertice(BymlFile::Type::Dictionary);

						BymlFile::Node PolytopeVerticeX(BymlFile::Type::Float, "X");
						PolytopeVerticeX.SetValue<float>(BfresModel->GetModels()[0].Vertices[i][VerticeIndex * 3]);

						BymlFile::Node PolytopeVerticeY(BymlFile::Type::Float, "Y");
						PolytopeVerticeY.SetValue<float>(BfresModel->GetModels()[0].Vertices[i][VerticeIndex * 3 + 1]);

						BymlFile::Node PolytopeVerticeZ(BymlFile::Type::Float, "Z");
						PolytopeVerticeZ.SetValue<float>(BfresModel->GetModels()[0].Vertices[i][VerticeIndex * 3 + 2]);

						PolytopeVertice.AddChild(PolytopeVerticeX);
						PolytopeVertice.AddChild(PolytopeVerticeY);
						PolytopeVertice.AddChild(PolytopeVerticeZ);

						PolytopeVertices.AddChild(PolytopeVertice);
					}
				}
			}

			PolytopeShapeDict.AddChild(PolytopeVertices);
			PolytopeArrayNode.AddChild(PolytopeShapeDict);
			ShapeParamByml.GetNodes().push_back(PolytopeArrayNode);

			SarcFile ActorPackSarc("MapEditor_Bake_Collision.pack");

			ActorPackSarc.GetEntry("Phive/ShapeParam/MapEditor_Bake_Collision__Physical.phive__ShapeParam.bgyml").Bytes = ShapeParamByml.ToBinary(BymlFile::TableGeneration::Auto);

			std::vector<std::string> BymlsToReplaceData = { "Component/ELink/MapEditor_Bake_Collision.engine__component__ELinkParam.bgyml", "Component/Physics/MapEditor_Bake_Collision.engine__component__PhysicsParam.bgyml", "Actor/MapEditor_Bake_Collision.engine__actor__ActorParam.bgyml", "Phive/ControllerSetParam/MapEditor_Bake_Collision.phive__ControllerSetParam.bgyml", "Phive/ShapeParam/MapEditor_Bake_Collision__Physical.phive__ShapeParam.bgyml" };
			for (int i = 0; i < BymlsToReplaceData.size(); i++) {
				BymlFile File(ActorPackSarc.GetEntry(BymlsToReplaceData[i]).Bytes);
				for (BymlFile::Node& Nodes : File.GetNodes())
				{
					ReplaceStringInBymlNodes(&Nodes, "MapEditor_Bake_Collision", ExportedActor.GetGyml());
				}
				ActorPackSarc.GetEntry(BymlsToReplaceData[i]).Bytes = File.ToBinary(BymlFile::TableGeneration::Auto);
			}

			for (SarcFile::Entry& Value : ActorPackSarc.GetEntries())
			{
				Util::ReplaceString(Value.Name, "MapEditor_Bake_Collision", ExportedActor.GetGyml());
			}


			//ActorPackSarc.WriteToFile(Config::GetWorkingDirFile("Cache/MapEditor_Bake_Collision_Cube_" + std::to_string(ExportedExportedActor.GetSRTHash()) + ".pack"));
			//ZStdFile ActorPackSarcCompressed(Util::GetWorkingDirFile("Cache/MapEditor_Bake_Collision_RCube_" + std::to_string(actor.SRTHash) + ".pack"), ZStdMode::ZStdDictionary::PackZSDictionary, ZStdMode::ZStdOperation::Compress);
			Util::CreateDir(Path + "/Pack");
			Util::CreateDir(Path + "/Pack/Actor");
			//ActorPackSarcCompressed.Compress(ExportPath + "/Pack/Actor/MapEditor_Bake_Collision_RCube_" + std::to_string(actor.SRTHash) + ".pack.zs");
			ZStdFile::Compress(ActorPackSarc.ToBinary(), ZStdFile::Dictionary::Pack).WriteToFile(Path + "/Pack/Actor/" + ExportedActor.GetGyml() + ".pack.zs");
		}
	}

	BymlFile::Node ActorNode(BymlFile::Type::Dictionary);

	if (ExportedActor.GetType() == Actor::Type::Merged)
	{
		auto Iter = Config::MergedActorBymls.begin();
		std::advance(Iter, ExportedActor.GetMergedActorIndex());

		ExportedActor.GetTranslate().SetX(ExportedActor.GetTranslate().GetX() - Actors->at(Iter->first).GetTranslate().GetX());
		ExportedActor.GetTranslate().SetY(ExportedActor.GetTranslate().GetY() - Actors->at(Iter->first).GetTranslate().GetY());
		ExportedActor.GetTranslate().SetZ(ExportedActor.GetTranslate().GetZ() - Actors->at(Iter->first).GetTranslate().GetZ());
	}

	if (ExportedActor.IsBakeable())
	{
		BymlFile::Node Bakeable(BymlFile::Type::Bool, "Bakeable");
		Bakeable.SetValue<bool>(true);
		ActorNode.AddChild(Bakeable);
	}

	if (ExportedActor.GetExtraCreateRadius() != 0)
	{
		BymlFile::Node ExtraCreateRadius(BymlFile::Type::Float, "ExtraCreateRadius");
		ExtraCreateRadius.SetValue<float>(ExportedActor.GetExtraCreateRadius());
		ActorNode.AddChild(ExtraCreateRadius);
	}

	if (!ExportedActor.GetDynamic().DynamicString.empty() || !ExportedActor.GetDynamic().DynamicVector.empty())
	{
		BymlFile::Node Dynamic(BymlFile::Type::Dictionary, "Dynamic");
		for (auto const& [Key, Value] : ExportedActor.GetDynamic().DynamicString)
		{
			BymlFile::Node DynamicEntry(GetDynamicValueDataType(Value), Key);
			if (DynamicEntry.GetType() == BymlFile::Type::UInt32) DynamicEntry.GetType() = BymlFile::Type::Int32;

			switch (DynamicEntry.GetType())
			{
			case BymlFile::Type::StringIndex:
				DynamicEntry.SetValue<std::string>(Value);
				break;
			case BymlFile::Type::Float:
				DynamicEntry.SetValue<float>(Util::StringToNumber<float>(Value));
				break;
			case BymlFile::Type::UInt32:
				DynamicEntry.SetValue<uint32_t>(Util::StringToNumber<uint32_t>(Value));
				break;
			case BymlFile::Type::UInt64:
				DynamicEntry.SetValue<uint64_t>(Util::StringToNumber<uint64_t>(Value));
				break;
			case BymlFile::Type::Int32:
				DynamicEntry.SetValue<int32_t>(Util::StringToNumber<int32_t>(Value));
				break;
			case BymlFile::Type::Int64:
				DynamicEntry.SetValue<int64_t>(Util::StringToNumber<int64_t>(Value));
				break;
			case BymlFile::Type::Bool:
				DynamicEntry.SetValue<bool>(Value == "true");
				break;
			}
			Dynamic.AddChild(DynamicEntry);
		}
		for (auto const& [Key, Value] : ExportedActor.GetDynamic().DynamicVector)
		{
			WriteBymlVector(&Dynamic, Key, Value, Vector3F(0, 0, 0), true);
		}
		ActorNode.AddChild(Dynamic);
	}

	BymlFile::Node Gyml(BymlFile::Type::StringIndex, "Gyaml");
	Gyml.SetValue<std::string>(ExportedActor.GetGyml());
	ActorNode.AddChild(Gyml);

	BymlFile::Node Hash(BymlFile::Type::UInt64, "Hash");
	Hash.SetValue<uint64_t>(ExportedActor.GetHash());
	ActorNode.AddChild(Hash);

	if (ExportedActor.IsForceActive())
	{
		BymlFile::Node ForceActive(BymlFile::Type::Bool, "IsForceActive");
		ForceActive.SetValue<bool>(ExportedActor.IsForceActive());
		ActorNode.AddChild(ForceActive);
	}

	if (ExportedActor.IsInWater())
	{
		BymlFile::Node IsInWater(BymlFile::Type::Bool, "IsInWater");
		IsInWater.SetValue<bool>(ExportedActor.IsInWater());
		ActorNode.AddChild(IsInWater);
	}

	if (ExportedActor.IsPhysicsStable())
	{
		BymlFile::Node PhysicStable(BymlFile::Type::Bool, "IsPhysicsStable");
		PhysicStable.SetValue<bool>(ExportedActor.IsPhysicsStable());
		ActorNode.AddChild(PhysicStable);
	}

	if (!ExportedActor.GetLinks().empty())
	{
		BymlFile::Node Links(BymlFile::Type::Array, "Links");
		for (Actor::Link& Link : ExportedActor.GetLinks())
		{

			BymlFile::Node LinkDict(BymlFile::Type::Dictionary);

			BymlFile::Node LinkDst(BymlFile::Type::UInt64, "Dst");
			LinkDst.SetValue<uint64_t>(Link.Dst);
			LinkDict.AddChild(LinkDst);

			if (Link.Gyaml != "")
			{
				BymlFile::Node LinkGyaml(BymlFile::Type::StringIndex, "Gyaml");
				LinkGyaml.SetValue<std::string>(Link.Gyaml);
				LinkDict.AddChild(LinkGyaml);
			}

			BymlFile::Node LinkName(BymlFile::Type::StringIndex, "Name");
			LinkName.SetValue<std::string>(Link.Name);

			BymlFile::Node LinkSrc(BymlFile::Type::UInt64, "Src");
			LinkSrc.SetValue<uint64_t>(Link.Src);

			LinkDict.AddChild(LinkName);
			LinkDict.AddChild(LinkSrc);
			Links.AddChild(LinkDict);
		}
		ActorNode.AddChild(Links);
	}

	if (ExportedActor.GetMoveRadius() != 0)
	{
		BymlFile::Node MoveRadius(BymlFile::Type::Float, "MoveRadius");
		MoveRadius.SetValue<float>(ExportedActor.GetMoveRadius());
		ActorNode.AddChild(MoveRadius);
	}

	if (ExportedActor.GetName() != "")
	{
		BymlFile::Node Name(BymlFile::Type::StringIndex, "Name");
		Name.SetValue<std::string>(ExportedActor.GetName());
		ActorNode.AddChild(Name);
	}

	if (!ExportedActor.GetPhive().Placement.empty() || !ExportedActor.GetPhive().ConstraintLink.Owners.empty() || ExportedActor.GetPhive().ConstraintLink.ID != 0 || !ExportedActor.GetPhive().ConstraintLink.Refers.empty())
	{
		BymlFile::Node Phive(BymlFile::Type::Dictionary, "Phive");

		if (!ExportedActor.GetPhive().ConstraintLink.Owners.empty() || ExportedActor.GetPhive().ConstraintLink.ID != 0 || !ExportedActor.GetPhive().ConstraintLink.Refers.empty())
		{
			BymlFile::Node ConstraintLink(BymlFile::Type::Dictionary, "ConstraintLink");

			if (ExportedActor.GetPhive().ConstraintLink.ID != 0)
			{
				BymlFile::Node ID(BymlFile::Type::UInt64, "ID");
				ID.SetValue<uint64_t>(ExportedActor.GetPhive().ConstraintLink.ID);
				ConstraintLink.AddChild(ID);
			}

			if (!ExportedActor.GetPhive().ConstraintLink.Owners.empty())
			{
				BymlFile::Node Owners(BymlFile::Type::Array, "Owners");
				for (Actor::Phive::ConstraintLinkData::OwnerData& Owner : ExportedActor.GetPhive().ConstraintLink.Owners)
				{
					BymlFile::Node OwnerNode(BymlFile::Type::Dictionary);

					if (!Owner.BreakableData.empty())
					{
						BymlFile::Node BreakableDataNode(BymlFile::Type::Dictionary, "BreakableData");

						for (auto const& [Key, Value] : Owner.BreakableData)
						{
							BymlFile::Node BreakableDataEntry(GetDynamicValueDataType(Value), Key);
							switch (BreakableDataEntry.GetType())
							{
							case BymlFile::Type::StringIndex:
								BreakableDataEntry.SetValue<std::string>(Value);
								break;
							case BymlFile::Type::Float:
								BreakableDataEntry.SetValue<float>(Util::StringToNumber<float>(Value));
								break;
							case BymlFile::Type::UInt32:
								BreakableDataEntry.SetValue<uint32_t>(Util::StringToNumber<uint32_t>(Value));
								break;
							case BymlFile::Type::UInt64:
								BreakableDataEntry.SetValue<uint64_t>(Util::StringToNumber<uint64_t>(Value));
								break;
							case BymlFile::Type::Int32:
								BreakableDataEntry.SetValue<int32_t>(Util::StringToNumber<int32_t>(Value));
								break;
							case BymlFile::Type::Int64:
								BreakableDataEntry.SetValue<int64_t>(Util::StringToNumber<int64_t>(Value));
								break;
							case BymlFile::Type::Bool:
								BreakableDataEntry.SetValue<bool>(Value == "true");
								break;
							}
							BreakableDataNode.AddChild(BreakableDataEntry);
						}

						OwnerNode.AddChild(BreakableDataNode);
					}


					if (!Owner.ClusterData.empty())
					{
						BymlFile::Node ClusterDataNode(BymlFile::Type::Dictionary, "ClusterData");

						for (auto const& [Key, Value] : Owner.ClusterData)
						{
							BymlFile::Node ClusterDataEntry(GetDynamicValueDataType(Value), Key);
							switch (ClusterDataEntry.GetType())
							{
							case BymlFile::Type::StringIndex:
								ClusterDataEntry.SetValue<std::string>(Value);
								break;
							case BymlFile::Type::Float:
								ClusterDataEntry.SetValue<float>(Util::StringToNumber<float>(Value));
								break;
							case BymlFile::Type::UInt32:
								ClusterDataEntry.SetValue<uint32_t>(Util::StringToNumber<uint32_t>(Value));
								break;
							case BymlFile::Type::UInt64:
								ClusterDataEntry.SetValue<uint64_t>(Util::StringToNumber<uint64_t>(Value));
								break;
							case BymlFile::Type::Int32:
								ClusterDataEntry.SetValue<int32_t>(Util::StringToNumber<int32_t>(Value));
								break;
							case BymlFile::Type::Int64:
								ClusterDataEntry.SetValue<int64_t>(Util::StringToNumber<int64_t>(Value));
								break;
							case BymlFile::Type::Bool:
								ClusterDataEntry.SetValue<bool>(Value == "true");
								break;
							}
							ClusterDataNode.AddChild(ClusterDataEntry);
						}

						OwnerNode.AddChild(ClusterDataNode);
					}

					if (Owner.OwnerPose.Rotate.GetX() != 0 ||
						Owner.OwnerPose.Rotate.GetY() != 0 ||
						Owner.OwnerPose.Rotate.GetZ() != 0 ||
						Owner.OwnerPose.Translate.GetX() != 0 ||
						Owner.OwnerPose.Translate.GetY() != 0 ||
						Owner.OwnerPose.Translate.GetZ() != 0)
					{
						BymlFile::Node OwnerPoseNode(BymlFile::Type::Dictionary, "OwnerPose");

						Vector3F RotateVector(Util::DegreeToRadian(Owner.OwnerPose.Rotate.GetX()), Util::DegreeToRadian(Owner.OwnerPose.Rotate.GetY()), Util::DegreeToRadian(Owner.OwnerPose.Rotate.GetZ()));
						WriteBymlVector(&OwnerPoseNode, "Rotate", RotateVector, Vector3F(0, 0, 0), true);
						WriteBymlVector(&OwnerPoseNode, "Trans", Owner.OwnerPose.Translate, Vector3F(0, 0, 0), true);

						OwnerNode.AddChild(OwnerPoseNode);
					}

					if (!Owner.ParamData.empty())
					{
						BymlFile::Node ParamDataNode(BymlFile::Type::Dictionary, "ParamData");

						for (auto const& [Key, Value] : Owner.ParamData)
						{
							BymlFile::Node ParamDataEntry(GetDynamicValueDataType(Value), Key);
							switch (ParamDataEntry.GetType())
							{
							case BymlFile::Type::StringIndex:
								ParamDataEntry.SetValue<std::string>(Value);
								break;
							case BymlFile::Type::Float:
								ParamDataEntry.SetValue<float>(Util::StringToNumber<float>(Value));
								break;
							case BymlFile::Type::UInt32:
								ParamDataEntry.SetValue<uint32_t>(Util::StringToNumber<uint32_t>(Value));
								break;
							case BymlFile::Type::UInt64:
								ParamDataEntry.SetValue<uint64_t>(Util::StringToNumber<uint64_t>(Value));
								break;
							case BymlFile::Type::Int32:
								ParamDataEntry.SetValue<int32_t>(Util::StringToNumber<int32_t>(Value));
								break;
							case BymlFile::Type::Int64:
								ParamDataEntry.SetValue<int64_t>(Util::StringToNumber<int64_t>(Value));
								break;
							case BymlFile::Type::Bool:
								ParamDataEntry.SetValue<bool>(Value == "true");
								break;
							}
							ParamDataNode.AddChild(ParamDataEntry);
						}

						OwnerNode.AddChild(ParamDataNode);
					}

					BymlFile::Node PivotDataNode(BymlFile::Type::Dictionary, "PivotData");
					if (Owner.PivotData.Pivot.GetX() != 0 || Owner.PivotData.Pivot.GetY() != 0 || Owner.PivotData.Pivot.GetZ() != 0)
					{
						BymlFile::Node PivotDataAxisNode(BymlFile::Type::Int32, "Axis");
						PivotDataAxisNode.SetValue<int32_t>(Owner.PivotData.Axis);
						PivotDataNode.AddChild(PivotDataAxisNode);
						WriteBymlVector(&PivotDataNode, "Pivot", Owner.PivotData.Pivot, Vector3F(0, 0, 0), true);
					}
					OwnerNode.AddChild(PivotDataNode);

					BymlFile::Node ReferNode(BymlFile::Type::UInt64, "Refer");
					ReferNode.SetValue<uint64_t>(Owner.Refer);
					OwnerNode.AddChild(ReferNode);

					if (Owner.ReferPose.Rotate.GetX() != 0 ||
						Owner.ReferPose.Rotate.GetY() != 0 ||
						Owner.ReferPose.Rotate.GetZ() != 0 ||
						Owner.ReferPose.Translate.GetX() != 0 ||
						Owner.ReferPose.Translate.GetY() != 0 ||
						Owner.ReferPose.Translate.GetZ() != 0)
					{
						BymlFile::Node ReferPoseNode(BymlFile::Type::Dictionary, "ReferPose");

						Vector3F RotateVector(Util::DegreeToRadian(Owner.ReferPose.Rotate.GetX()), Util::DegreeToRadian(Owner.ReferPose.Rotate.GetY()), Util::DegreeToRadian(Owner.ReferPose.Rotate.GetZ()));
						WriteBymlVector(&ReferPoseNode, "Rotate", RotateVector, Vector3F(0, 0, 0), true);
						WriteBymlVector(&ReferPoseNode, "Trans", Owner.ReferPose.Translate, Vector3F(0, 0, 0), true);

						OwnerNode.AddChild(ReferPoseNode);
					}

					if (Owner.Type != "")
					{
						BymlFile::Node TypeNode(BymlFile::Type::StringIndex, "Type");
						TypeNode.SetValue<std::string>(Owner.Type);
						OwnerNode.AddChild(TypeNode);
					}

					if (!Owner.UserData.empty())
					{
						BymlFile::Node UserDataNode(BymlFile::Type::Dictionary, "UserData");

						for (auto const& [Key, Value] : Owner.UserData)
						{
							BymlFile::Node UserDataEntry(GetDynamicValueDataType(Value), Key);
							switch (UserDataEntry.GetType())
							{
							case BymlFile::Type::StringIndex:
								UserDataEntry.SetValue<std::string>(Value);
								break;
							case BymlFile::Type::Float:
								UserDataEntry.SetValue<float>(Util::StringToNumber<float>(Value));
								break;
							case BymlFile::Type::UInt32:
								UserDataEntry.SetValue<uint32_t>(Util::StringToNumber<uint32_t>(Value));
								break;
							case BymlFile::Type::UInt64:
								UserDataEntry.SetValue<uint64_t>(Util::StringToNumber<uint64_t>(Value));
								break;
							case BymlFile::Type::Int32:
								UserDataEntry.SetValue<int32_t>(Util::StringToNumber<int32_t>(Value));
								break;
							case BymlFile::Type::Int64:
								UserDataEntry.SetValue<int64_t>(Util::StringToNumber<int64_t>(Value));
								break;
							case BymlFile::Type::Bool:
								UserDataEntry.SetValue<bool>(Value == "true");
								break;
							}
							UserDataNode.AddChild(UserDataEntry);
						}

						OwnerNode.AddChild(UserDataNode);
					}
					Owners.AddChild(OwnerNode);
				}
				ConstraintLink.AddChild(Owners);
			}

			if (!ExportedActor.GetPhive().ConstraintLink.Refers.empty())
			{
				BymlFile::Node Refers(BymlFile::Type::Array, "Refers");
				for (Actor::Phive::ConstraintLinkData::ReferData Refer : ExportedActor.GetPhive().ConstraintLink.Refers)
				{
					BymlFile::Node ReferNode(BymlFile::Type::Dictionary);

					BymlFile::Node Owner(BymlFile::Type::UInt64, "Owner");
					Owner.SetValue<uint64_t>(Refer.Owner);

					BymlFile::Node Type(BymlFile::Type::StringIndex, "Type");
					Type.SetValue<std::string>(Refer.Type);

					ReferNode.AddChild(Owner);
					ReferNode.AddChild(Type);

					Refers.AddChild(ReferNode);
				}
				ConstraintLink.AddChild(Refers);
			}

			Phive.AddChild(ConstraintLink);
		}

		if (!ExportedActor.GetPhive().Placement.empty())
		{
			BymlFile::Node Placement(BymlFile::Type::Dictionary, "Placement");

			for (auto const& [Key, Value] : ExportedActor.GetPhive().Placement)
			{
				BymlFile::Node PlacementEntry(GetDynamicValueDataType(Value), Key);
				if (Key == "GroupID")
				{
					PlacementEntry.GetType() = BymlFile::Type::Int32;
				}
				switch (PlacementEntry.GetType())
				{
				case BymlFile::Type::StringIndex:
					PlacementEntry.SetValue<std::string>(Value);
					break;
				case BymlFile::Type::Float:
					PlacementEntry.SetValue<float>(Util::StringToNumber<float>(Value));
					break;
				case BymlFile::Type::UInt32:
					PlacementEntry.SetValue<uint32_t>(Util::StringToNumber<uint32_t>(Value));
					break;
				case BymlFile::Type::UInt64:
					PlacementEntry.SetValue<uint64_t>(Util::StringToNumber<uint64_t>(Value));
					break;
				case BymlFile::Type::Int32:
					PlacementEntry.SetValue<int32_t>(Util::StringToNumber<int32_t>(Value));
					break;
				case BymlFile::Type::Int64:
					PlacementEntry.SetValue<int64_t>(Util::StringToNumber<int64_t>(Value));
					break;
				}
				Placement.AddChild(PlacementEntry);
			}
			Phive.AddChild(Placement);
		}

		if (!ExportedActor.GetPhive().Rails.empty())
		{
			BymlFile::Node Rails(BymlFile::Type::Array, "Rails");

			for (Actor::Phive::RailData& Rail : ExportedActor.GetPhive().Rails)
			{
				BymlFile::Node RailNode(BymlFile::Type::Dictionary);

				BymlFile::Node IsClosed(BymlFile::Type::Bool, "IsClosed");
				IsClosed.SetValue<bool>(Rail.IsClosed);
				RailNode.AddChild(IsClosed);

				if (!Rail.Nodes.empty())
				{
					BymlFile::Node Nodes(BymlFile::Type::Array, "Nodes");

					for (Actor::Phive::RailData::Node RailNode : Rail.Nodes)
					{
						BymlFile::Node RailDict(BymlFile::Type::Dictionary);
						WriteBymlVector(&RailDict, RailNode.Key, RailNode.Value, Vector3F(0, 0, 0), true);
						Nodes.AddChild(RailDict);
					}

					RailNode.AddChild(Nodes);
				}

				if (Rail.Type != "")
				{
					BymlFile::Node Type(BymlFile::Type::StringIndex, "Type");
					Type.SetValue<std::string>(Rail.Type);
					RailNode.AddChild(Type);
				}

				Rails.AddChild(RailNode);
			}

			Phive.AddChild(Rails);
		}

		if (ExportedActor.GetPhive().RopeHeadLink.ID != 0)
		{
			BymlFile::Node RopeHeadLinkNode(BymlFile::Type::Dictionary, "RopeHeadLink");

			BymlFile::Node ID(BymlFile::Type::UInt64, "ID");
			ID.SetValue<uint64_t>(ExportedActor.GetPhive().RopeHeadLink.ID);
			RopeHeadLinkNode.AddChild(ID);

			if (!ExportedActor.GetPhive().RopeHeadLink.Owners.empty())
			{
				BymlFile::Node Owners(BymlFile::Type::Array, "Owners");

				for (uint64_t Owner : ExportedActor.GetPhive().RopeHeadLink.Owners)
				{
					BymlFile::Node OwnerNode(BymlFile::Type::Dictionary);

					BymlFile::Node Refer(BymlFile::Type::UInt64, "Refer");
					Refer.SetValue<uint64_t>(Owner);
					OwnerNode.AddChild(Refer);

					Owners.AddChild(OwnerNode);
				}
				RopeHeadLinkNode.AddChild(Owners);
			}

			if (!ExportedActor.GetPhive().RopeHeadLink.Refers.empty())
			{
				BymlFile::Node Refers(BymlFile::Type::Array, "Refers");

				for (uint64_t Refer : ExportedActor.GetPhive().RopeHeadLink.Refers)
				{
					BymlFile::Node ReferNode(BymlFile::Type::Dictionary);

					BymlFile::Node Owner(BymlFile::Type::UInt64, "Owner");
					Owner.SetValue<uint64_t>(Refer);
					ReferNode.AddChild(Owner);

					Refers.AddChild(ReferNode);
				}
				RopeHeadLinkNode.AddChild(Refers);
			}

			Phive.AddChild(RopeHeadLinkNode);
		}

		if (ExportedActor.GetPhive().RopeTailLink.ID != 0)
		{
			BymlFile::Node RopeTailLinkNode(BymlFile::Type::Dictionary, "RopeTailLink");

			BymlFile::Node ID(BymlFile::Type::UInt64, "ID");
			ID.SetValue<uint64_t>(ExportedActor.GetPhive().RopeTailLink.ID);
			RopeTailLinkNode.AddChild(ID);

			if (!ExportedActor.GetPhive().RopeTailLink.Owners.empty())
			{
				BymlFile::Node Owners(BymlFile::Type::Array, "Owners");

				for (uint64_t Owner : ExportedActor.GetPhive().RopeTailLink.Owners)
				{
					BymlFile::Node OwnerNode(BymlFile::Type::Dictionary);

					BymlFile::Node Refer(BymlFile::Type::UInt64, "Refer");
					Refer.SetValue<uint64_t>(Owner);
					OwnerNode.AddChild(Refer);

					Owners.AddChild(OwnerNode);
				}
				RopeTailLinkNode.AddChild(Owners);
			}

			if (!ExportedActor.GetPhive().RopeTailLink.Refers.empty())
			{
				BymlFile::Node Refers(BymlFile::Type::Array, "Refers");

				for (uint64_t Refer : ExportedActor.GetPhive().RopeTailLink.Refers)
				{
					BymlFile::Node ReferNode(BymlFile::Type::Dictionary);

					BymlFile::Node Owner(BymlFile::Type::UInt64, "Owner");
					Owner.SetValue<uint64_t>(Refer);
					ReferNode.AddChild(Owner);

					Refers.AddChild(ReferNode);
				}
				RopeTailLinkNode.AddChild(Refers);
			}

			Phive.AddChild(RopeTailLinkNode);
		}

		ActorNode.AddChild(Phive);
	}

	if (!ExportedActor.GetPresence().empty())
	{
		BymlFile::Node Presence(BymlFile::Type::Dictionary, "Presence");
		for (auto const& [Key, Value] : ExportedActor.GetPresence())
		{
			BymlFile::Node PresenceEntry(GetDynamicValueDataType(Value), Key);
			switch (PresenceEntry.GetType())
			{
			case BymlFile::Type::StringIndex:
				PresenceEntry.SetValue<std::string>(Value);
				break;
			case BymlFile::Type::Float:
				PresenceEntry.SetValue<float>(Util::StringToNumber<float>(Value));
				break;
			case BymlFile::Type::UInt32:
				PresenceEntry.SetValue<uint32_t>(Util::StringToNumber<uint32_t>(Value));
				break;
			case BymlFile::Type::UInt64:
				PresenceEntry.SetValue<uint64_t>(Util::StringToNumber<uint64_t>(Value));
				break;
			case BymlFile::Type::Int32:
				PresenceEntry.SetValue<int32_t>(Util::StringToNumber<int32_t>(Value));
				break;
			case BymlFile::Type::Int64:
				PresenceEntry.SetValue<int64_t>(Util::StringToNumber<int64_t>(Value));
				break;
			case BymlFile::Type::Bool:
				PresenceEntry.SetValue<bool>(Value == "true");
				break;
			}
			Presence.AddChild(PresenceEntry);
		}
		ActorNode.AddChild(Presence);
	}

	if (!ExportedActor.GetRails().empty())
	{
		BymlFile::Node Rails(BymlFile::Type::Array, "Rails");
		for (Actor::Rail Rail : ExportedActor.GetRails())
		{
			BymlFile::Node RailDict(BymlFile::Type::Dictionary);

			BymlFile::Node RailDst(BymlFile::Type::UInt64, "Dst");
			RailDst.SetValue<uint64_t>(Rail.Dst);
			RailDict.AddChild(RailDst);

			if (Rail.Gyaml != "")
			{
				BymlFile::Node RailGyaml(BymlFile::Type::StringIndex, "Gyaml");
				RailGyaml.SetValue<std::string>(Rail.Gyaml);
				RailDict.AddChild(RailGyaml);
			}

			BymlFile::Node RailName(BymlFile::Type::StringIndex, "Name");
			RailName.SetValue<std::string>(Rail.Name);

			RailDict.AddChild(RailName);
			Rails.AddChild(RailDict);
		}
		ActorNode.AddChild(Rails);
	}

	Vector3F RotateVector(Util::DegreeToRadian(ExportedActor.GetRotate().GetX()), Util::DegreeToRadian(ExportedActor.GetRotate().GetY()), Util::DegreeToRadian(ExportedActor.GetRotate().GetZ()));
	WriteBymlVector(&ActorNode, "Rotate", RotateVector, Vector3F(0, 0, 0), false);

	BymlFile::Node SRTHash(BymlFile::Type::UInt32, "SRTHash");
	SRTHash.SetValue<uint32_t>(ExportedActor.GetSRTHash());
	ActorNode.AddChild(SRTHash);

	WriteBymlVector(&ActorNode, "Scale", ExportedActor.GetScale(), Vector3F(1, 1, 1), false);
	WriteBymlVector(&ActorNode, "Translate", ExportedActor.GetTranslate(), Vector3F(0, 0, 0), false);

	if (ExportedActor.IsTurnActorNearEnemy())
	{
		BymlFile::Node TurnActorNearEnemy(BymlFile::Type::Bool, "TurnActorNearEnemy");
		TurnActorNearEnemy.SetValue<bool>(ExportedActor.IsTurnActorNearEnemy());
		ActorNode.AddChild(TurnActorNearEnemy);
	}

	return ActorNode;
}

void Exporter::Export(std::vector<Actor>* Actors, std::string Path, bool Save)
{
	std::string OldPath;

	if (!Save)
	{
		OldPath = Path;
		Path = Config::GetWorkingDirFile("Save");
		EditorConfig::Save();
	}

	if (Config::Key.length() == 0)
	{
		goto Copy;
	}

	{
		MapConfig::Save(Actors);

		BymlFile& StaticByml = Config::StaticActorsByml;
		BymlFile& DynamicByml = Config::DynamicActorsByml;

		StaticByml.GetNode("Actors")->GetChildren().clear();
		DynamicByml.GetNode("Actors")->GetChildren().clear();

		for (auto& [Key, Val] : Config::MergedActorBymls)
		{
			if (Val.HasChild("Actors"))
			{
				Val.GetNode("Actors")->GetChildren().clear();
			}
		}
		 
		for (Actor ExportedActor : *Actors)
		{
			if (ExportedActor.GetType() == Actor::Type::Static)
			{
				StaticByml.GetNode("Actors")->AddChild(ActorToByml(Path, true, Actors, ExportedActor));
			}
			else if(ExportedActor.GetType() == Actor::Type::Dynamic)
			{
				DynamicByml.GetNode("Actors")->AddChild(ActorToByml(Path, true, Actors, ExportedActor));
			}
			else //Merged
			{
				auto Iter = Config::MergedActorBymls.begin();
				std::advance(Iter, ExportedActor.GetMergedActorIndex());

				Iter->second.GetNode("Actors")->AddChild(ActorToByml(Path, true, Actors, ExportedActor));
			}
		}

		ZStdFile::Compress(StaticByml.ToBinary(BymlFile::TableGeneration::Auto), ZStdFile::Dictionary::BcettByaml).WriteToFile(Path + "/" + Config::BancPrefix + Config::Key + "_Static.bcett.byml.zs");
		ZStdFile::Compress(DynamicByml.ToBinary(BymlFile::TableGeneration::Auto), ZStdFile::Dictionary::BcettByaml).WriteToFile(Path + "/" + Config::BancPrefix + Config::Key + "_Dynamic.bcett.byml.zs");

		for (auto& [Key, Val] : Config::MergedActorBymls)
		{
			ZStdFile::Compress(Val.ToBinary(BymlFile::TableGeneration::Auto), ZStdFile::Dictionary::BcettByaml).WriteToFile(Path + "/" + Actors->at(Key).GetDynamic().DynamicString["BancPath"] + ".zs");
		}
	}

Copy:

	if (!Save)
	{
		CreateExportOnlyFiles(Actors, OldPath);

		Util::CopyDirectoryRecursively(Config::GetWorkingDirFile("Save"), OldPath);
	}
}