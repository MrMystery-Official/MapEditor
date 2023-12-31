#pragma once

#define PI 3.14159

#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace Util
{
	float RadianToDegree(float Radian);
	float DegreeToRadian(float Degree);
	void CreateDir(std::string Path);

	template<class T> T StringToNumber(std::string Value);
	bool FileExists(std::string Path);
	void CopyFileToDest(std::string Source, std::string Dest);
	bool ReplaceString(std::string& str, const std::string& from, const std::string& to);
	bool EndsWith(std::string const& value, std::string const& ending);
	void CopyDirectoryRecursively(const std::string& SourcePath, const std::string& DestinationPath);
};