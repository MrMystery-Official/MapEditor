#pragma once

#include "Actor.h"

namespace TemplateMgr
{
	class Template
	{
	public:
		Template(std::string Name);
		Template(std::string Name, std::vector<Actor> Actors);

		std::string& GetName();
		std::vector<Actor>& GetActors();

		void Paste(Vector3F BaseLocation, std::vector<Actor>* Actors);
	private:
		std::string m_Name;
		std::vector<Actor> m_Actors;
	};

	extern std::vector<Template> Templates;

	void Initialize();
	void Save();

	Template* GetTemplate(std::string Name);
	std::vector<Template>& GetTemplates();
	bool HasTemplate(std::string Name);
	
	void AddTemplate(std::string Name, std::vector<Actor> Actors);
};