#include "Actor.h"
#include "Config.h"

/* Actor Model Library - Start */
std::map<std::string, BfresFile> ActorModelLibrary::Models;

void ActorModelLibrary::Initialize()
{
	ActorModelLibrary::Models.insert({ "None", BfresFile::CreateDefaultModel("None", 0, 0, 255, 255) });
	ActorModelLibrary::Models.insert({ "Area", BfresFile::CreateDefaultModel("Area", 0, 255, 0, 50) });
	ActorModelLibrary::Models.insert({ "ForbidArea", BfresFile::CreateDefaultModel("ForbidArea", 255, 0, 0, 50) });
	ActorModelLibrary::Models.insert({ "Collision", BfresFile::CreateDefaultModel("Collision", 255, 255, 0, 50) });
}

bool ActorModelLibrary::IsModelLoaded(std::string ModelName)
{
	return ActorModelLibrary::Models.count(ModelName);
}

BfresFile* ActorModelLibrary::GetModel(std::string ModelName)
{
	if (!ActorModelLibrary::IsModelLoaded(ModelName)) 
		ActorModelLibrary::Models.insert({ ModelName, BfresFile(Config::GetBfresFile(ModelName + ".bfres")) });
	return &ActorModelLibrary::Models[ModelName];
}

std::map<std::string, BfresFile>& ActorModelLibrary::GetModels()
{
	return ActorModelLibrary::Models;
}

/* Actor Model Library - End */

std::string& Actor::GetGyml()
{
	return this->m_Gyml;
}
void Actor::SetGyml(std::string Gyml)
{
	this->m_Gyml = Gyml;
}

Actor::Type& Actor::GetType()
{
	return this->m_Type;
}
void Actor::SetType(Actor::Type Type)
{
	this->m_Type = Type;
}

bool& Actor::IsBakeable()
{
	return this->m_Bakeable;
}
void Actor::SetBakeable(bool Bakeable)
{
	this->m_Bakeable = Bakeable;
}

bool& Actor::IsPhysicsStable()
{
	return this->m_PhysicsStable;
}
void Actor::SetPhysicsStable(bool Stable)
{
	this->m_PhysicsStable = Stable;
}

bool& Actor::IsForceActive()
{
	return this->m_ForceActive;
}
void Actor::SetForceActive(bool ForceActive)
{
	this->m_ForceActive = ForceActive;
}

bool& Actor::IsCollisionClimbable()
{
	return this->m_CollisionClimbable;
}
void Actor::SetCollisionClimbable(bool Climbable)
{
	this->m_CollisionClimbable = Climbable;
}

std::string& Actor::GetCollisionFile()
{
	return this->m_CollisionFile;
}
void Actor::SetCollisionFile(std::string Path)
{
	this->m_CollisionFile = Path;
}

uint64_t& Actor::GetHash()
{
	return this->m_Hash;
}
void Actor::SetHash(uint64_t Hash)
{
	this->m_Hash = Hash;
}

uint32_t& Actor::GetSRTHash()
{
	return this->m_SRTHash;
}
void Actor::SetSRTHash(uint32_t SRTHash)
{
	this->m_SRTHash = SRTHash;
}

Vector3F& Actor::GetTranslate()
{
	return this->m_Translate;
}
void Actor::SetTranslate(Vector3F Translate)
{
	this->m_Translate = Translate;
}

Vector3F& Actor::GetRotate()
{
	return this->m_Rotate;
}
void Actor::SetRotate(Vector3F Rotate)
{
	this->m_Rotate = Rotate;
}

Vector3F& Actor::GetScale()
{
	return this->m_Scale;
}
void Actor::SetScale(Vector3F Scale)
{
	this->m_Scale = Scale;
}

BfresFile* Actor::GetModel()
{
	return this->m_Model;
}
void Actor::SetModel(BfresFile* Model)
{
	this->m_Model = Model;
}

std::string& Actor::GetName()
{
	return this->m_Name;
}
void Actor::SetName(std::string Name)
{
	this->m_Name = Name;
}

Actor::Dynamic& Actor::GetDynamic()
{
	return this->m_Dynamic;
}
void Actor::AddDynamic(std::string Key, std::string Value)
{
	this->m_Dynamic.DynamicString.insert({ Key, Value });
}
void Actor::AddDynamic(std::string Key, Vector3F Value)
{
	this->m_Dynamic.DynamicVector.insert({ Key, Value });
}
/*
void Actor::RemoveDynamic(std::string Key)
{
	this->m_Dynamic.erase(Key);
}
*/

std::map<std::string, std::string>& Actor::GetPresence()
{
	return this->m_Presence;
}
void Actor::AddPresence(std::string Key, std::string Value)
{
	this->m_Presence.insert({ Key, Value });
}
void Actor::RemovePresence(std::string Key)
{
	this->m_Presence.erase(Key);
}

std::vector<Actor::Rail>& Actor::GetRails()
{
	return this->m_Rails;
}
void Actor::AddRail(Actor::Rail Rail)
{
	this->m_Rails.push_back(Rail);
}

std::vector<Actor::Link>& Actor::GetLinks()
{
	return this->m_Links;
}
void Actor::AddLink(Actor::Link Link)
{
	this->m_Links.push_back(Link);
}

Actor::Phive& Actor::GetPhive()
{
	return this->m_Phive;
}

float& Actor::GetMoveRadius()
{
	return this->m_MoveRadius;
}
void Actor::SetMoveRadius(float MoveRadius)
{
	this->m_MoveRadius = MoveRadius;
}

float& Actor::GetExtraCreateRadius()
{
	return this->m_ExtraCreateRadius;
}
void Actor::SetExtraCreateRadius(float ExtraCreateRadius)
{
	this->m_ExtraCreateRadius = ExtraCreateRadius;
}

bool& Actor::IsTurnActorNearEnemy()
{
	return this->m_TurnActorNearEnemy;
}
void Actor::SetTurnActorNearEnemy(bool TurnActorNearEnemy)
{
	this->m_TurnActorNearEnemy = TurnActorNearEnemy;
}

bool& Actor::IsInWater()
{
	return this->m_InWater;
}
void Actor::SetInWater(bool InWater)
{
	this->m_InWater = InWater;
}

std::string& Actor::GetCategory()
{
	return this->m_Category;
}
void Actor::SetCategory(std::string Category)
{
	this->m_Category = Category;
}

uint32_t& Actor::GetCollisionSRTHash()
{
	return this->m_CollisionSRTHash;
}
void Actor::SetCollisionSRTHash(uint32_t Hash)
{
	this->m_CollisionSRTHash = Hash;
}

uint32_t& Actor::GetMergedActorIndex()
{
	return this->m_MergedActorIndex;
}
void Actor::SetMergedActorIndex(uint32_t Index)
{
	this->m_MergedActorIndex = Index;
}