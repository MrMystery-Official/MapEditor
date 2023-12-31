#include "Util.h"

float Util::RadianToDegree(float Radian)
{
	return Radian * (180 / PI);
}

float Util::DegreeToRadian(float Degree)
{
	return Degree * (PI / 180);
}

void Util::CreateDir(std::string Path)
{
	std::filesystem::create_directory(Path);
}

template<> uint32_t Util::StringToNumber(std::string Value)
{
	uint32_t Number;
	std::istringstream StringStream(Value);
	StringStream >> Number;
	return Number;
}

template<> uint64_t Util::StringToNumber(std::string Value)
{
	uint64_t Number;
	std::istringstream StringStream(Value);
	StringStream >> Number;
	return Number;
}

template<> int32_t Util::StringToNumber(std::string Value)
{
	int32_t Number;
	std::istringstream StringStream(Value);
	StringStream >> Number;
	return Number;
}

template<> int64_t Util::StringToNumber(std::string Value)
{
	int64_t Number;
	std::istringstream StringStream(Value);
	StringStream >> Number;
	return Number;
}

template<> float Util::StringToNumber(std::string Value)
{
	float Number;
	std::istringstream StringStream(Value);
	StringStream >> Number;
	return Number;
}

bool Util::FileExists(std::string Path)
{
	struct stat buffer;
	return (stat(Path.c_str(), &buffer) == 0);
}

void Util::CopyFileToDest(std::string Source, std::string Dest)
{
	std::ifstream src(Source, std::ios::binary);
	std::ofstream dst(Dest, std::ios::binary);
	if (!dst)
	{
		// If the destination file doesn't exist, create it
		dst.open(Dest, std::ios::binary);
		if (!dst)
		{
			std::cerr << "ERROR: Could not create or open destination file!\n";
			return;
		}
	}

	dst << src.rdbuf();

	src.close();
	dst.close();
}

bool Util::ReplaceString(std::string& str, const std::string& from, const std::string& to)
{
	size_t startPos = 0;
	while ((startPos = str.find(from, startPos)) != std::string::npos)
	{
		str.replace(startPos, from.length(), to);
		startPos += to.length(); // In case the replacement string contains the target substring
	}
	return true;
}

bool Util::EndsWith(std::string const& value, std::string const& ending)
{
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

void Util::CopyDirectoryRecursively(const std::string& SourcePath, const std::string& DestinationPath)
{
	for (const auto& Entry : std::filesystem::directory_iterator(SourcePath))
	{
		if (Entry.is_directory() && !std::filesystem::is_empty(Entry))
		{
			std::filesystem::copy(Entry, DestinationPath + "/" + Entry.path().filename().string(), std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);
		}
	}
}