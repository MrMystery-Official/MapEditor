#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <zstd.h>
#include <map>

#include "BinaryVectorReader.h"
#include "Util.h"
#include "Config.h"

class TextureToGo
{
public:
	void (*DecompressFunction)(unsigned int, unsigned int, std::vector<unsigned char>&, std::vector<unsigned char>&, TextureToGo* TexToGo) = nullptr;

	std::vector<unsigned char>& GetPixels();
	uint16_t& GetWidth();
	uint16_t& GetHeight();
	uint16_t& GetDepth();
	uint16_t& GetFormat();
	uint8_t& GetMipMapCount();
	bool& IsTransparent();
	bool& IsFail();

	TextureToGo(std::string Path, std::vector<unsigned char> Bytes);
	TextureToGo(std::string Path);
	TextureToGo() {
		this->m_Fail = true;
	}
private:
	struct SurfaceInfo
	{
		uint16_t MipMapLevel;
		uint8_t ArrayLevel;
		uint8_t SurfaceCount;
		uint32_t Size;
	};

	std::vector<unsigned char> m_Pixels;
	uint16_t m_Width;
	uint16_t m_Height;
	uint16_t m_Depth;
	uint8_t m_MipMapCount;
	uint16_t m_Format;
	bool m_Transparent = false;
	bool m_Fail = false;
};

namespace TextureToGoLibrary
{
	extern std::map<std::string, TextureToGo> Textures;

	bool IsTextureLoaded(std::string Name);
	TextureToGo* GetTexture(std::string Name);
	std::map<std::string, TextureToGo>& GetTextures();
};