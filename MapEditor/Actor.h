#pragma once

#include <string>
#include <vector>
#include <map>
#include "Bfres.h"
#include "Vector3F.h"

namespace ActorModelLibrary
{
	extern std::map<std::string, BfresFile> Models;

	void Initialize();
	bool IsModelLoaded(std::string ModelName);
	BfresFile* GetModel(std::string ModelName);
	std::map<std::string, BfresFile>& GetModels();
};

class Actor
{
public:
	enum class Type : uint8_t
	{
		Static = 0,
		Dynamic = 1,
		Merged = 2
	};

	struct Rail
	{
		uint64_t Dst;
		std::string Gyaml = "";
		std::string Name = "";
	};

	struct Link
	{
		uint64_t Dst;
		std::string Gyaml = "";
		std::string Name = "";
		uint64_t Src;
	};

	struct Phive
	{
		struct ConstraintLinkData
		{
			struct ReferData
			{
				uint64_t Owner;
				std::string Type;
			};

			struct OwnerData
			{
				struct OwnerPoseData
				{
					Vector3F Rotate;
					Vector3F Translate;
				};

				struct PivotDataStruct
				{
					int32_t Axis = 0;
					Vector3F Pivot;
				};

				struct ReferPoseData
				{
					Vector3F Rotate;
					Vector3F Translate;
				};

				std::map<std::string, std::string> BreakableData;
				std::map<std::string, std::string> ClusterData;
				OwnerPoseData OwnerPose;
				std::map<std::string, std::string> ParamData;
				PivotDataStruct PivotData;
				uint64_t Refer;
				ReferPoseData ReferPose;
				std::string Type = "";
				std::map<std::string, std::string> UserData;
			};

			uint64_t ID = 0;
			std::vector<OwnerData> Owners;
			std::vector<ReferData> Refers;
		};

		struct RopeLinkData
		{
			uint64_t ID = 0;
			std::vector<uint64_t> Owners;
			std::vector<uint64_t> Refers;
		};

		struct RailData
		{
			struct Node
			{
				std::string Key;
				Vector3F Value;
			};

			bool IsClosed = false;
			std::vector<Node> Nodes;
			std::string Type = "";
		};

		std::map<std::string, std::string> Placement;
		ConstraintLinkData ConstraintLink;
		RopeLinkData RopeHeadLink;
		RopeLinkData RopeTailLink;
		std::vector<RailData> Rails;
	};

	struct Dynamic
	{
		std::map<std::string, std::string> DynamicString;
		std::map<std::string, Vector3F> DynamicVector;
	};

	Actor::Type& GetType();
	void SetType(Actor::Type Type);

	std::string& GetGyml();
	void SetGyml(std::string Gyml);

	bool& IsBakeable();
	void SetBakeable(bool Bakeable);

	bool& IsPhysicsStable();
	void SetPhysicsStable(bool Stable);

	bool& IsForceActive();
	void SetForceActive(bool ForceActive);

	bool& IsCollisionClimbable();
	void SetCollisionClimbable(bool Climbable);

	std::string& GetCollisionFile();
	void SetCollisionFile(std::string Path);

	uint64_t& GetHash();
	void SetHash(uint64_t Hash);

	uint32_t& GetSRTHash();
	void SetSRTHash(uint32_t SRTHash);

	Vector3F& GetTranslate();
	void SetTranslate(Vector3F Translate);

	Vector3F& GetRotate();
	void SetRotate(Vector3F Rotate);

	Vector3F& GetScale();
	void SetScale(Vector3F Scale);

	BfresFile* GetModel();
	void SetModel(BfresFile* Model);

	std::string& GetName();
	void SetName(std::string Name);

	Dynamic& GetDynamic();
	void AddDynamic(std::string Key, std::string Value);
	void AddDynamic(std::string Key, Vector3F Value);
	//void RemoveDynamic(std::string Key);

	std::map<std::string, std::string>& GetPresence();
	void AddPresence(std::string Key, std::string Value);
	void RemovePresence(std::string Key);

	std::vector<Actor::Rail>& GetRails();
	void AddRail(Actor::Rail Rail);

	std::vector<Actor::Link>& GetLinks();
	void AddLink(Actor::Link Link);

	Phive& GetPhive();

	float& GetMoveRadius();
	void SetMoveRadius(float MoveRadius);

	float& GetExtraCreateRadius();
	void SetExtraCreateRadius(float ExtraCreateRadius);

	bool& IsTurnActorNearEnemy();
	void SetTurnActorNearEnemy(bool TurnActorNearEnemy);

	bool& IsInWater();
	void SetInWater(bool InWater);

	std::string& GetCategory();
	void SetCategory(std::string Category);

	uint32_t& GetCollisionSRTHash();
	void SetCollisionSRTHash(uint32_t Hash);

	uint32_t& GetMergedActorIndex();
	void SetMergedActorIndex(uint32_t Index);

	uint32_t& GetMergedActorParentIndex();
	void SetMergedActorParentIndex(uint32_t Index);

	bool& IsPhysicsObject();
	void SetPhysicsObject(bool PhysicsObject);
private:
	/* TotK Engine */
	Actor::Type m_Type; //Required
	std::string m_Gyml; //Required
	bool m_Bakeable = false; //Not required
	bool m_PhysicsStable = false; //Not reqiured
	bool m_ForceActive = false; //Not required
	uint64_t m_Hash; //Required
	uint32_t m_SRTHash; //Required
	Vector3F m_Rotate = Vector3F(0, 0, 0); //Not required
	Vector3F m_Scale = Vector3F(1, 1, 1); //Not required
	std::string m_Name = ""; //Not required
	std::map<std::string, std::string> m_Presence; //Not required
	std::vector<Rail> m_Rails; //Not required
	std::vector<Link> m_Links; //Not rquired
	float m_MoveRadius = 0.0f; //Not required
	float m_ExtraCreateRadius = 0.0f; //Not required
	bool m_TurnActorNearEnemy = false; //Not required
	bool m_InWater = false; //Not required
	std::string m_Category = ""; //Not required
	Dynamic m_Dynamic; //Not required
	Vector3F m_Translate = Vector3F(0, 0, 0); //Not required

	/* Editor */
	BfresFile* m_Model; //Technically not required, but edior will crash if not set
	bool m_PhysicsObject = false;

	/* Physics Engine */
	Phive m_Phive; //Technically not required, but TotK doesn't like actors without Phive data:(
	bool m_CollisionClimbable = false; //Not reqiured
	std::string m_CollisionFile = ""; //Not required
	uint32_t m_CollisionSRTHash = 0; //Not required

	/* Merged actor */
	uint32_t m_MergedActorIndex = -1;
	uint32_t m_MergedActorParentIndex = -1;
};