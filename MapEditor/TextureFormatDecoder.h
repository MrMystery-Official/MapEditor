#pragma once

#include <vector>
#include "TextureToGo.h"

namespace TextureFormatDecoder
{
	void DecodeBC1(unsigned int Width, unsigned int Height, std::vector<unsigned char>& Data, std::vector<unsigned char>& Dest, TextureToGo* TexToGo);
	void DecodeBC3SRG(unsigned int Width, unsigned int Height, std::vector<unsigned char>& Data, std::vector<unsigned char>& Dest, TextureToGo* TexToGo);
	void DecodeASTC8x8UNorm(unsigned int Width, unsigned int Height, std::vector<unsigned char>& Data, std::vector<unsigned char>& Dest, TextureToGo* TexToGo);
	void DecodeASTC4x4UNorm(unsigned int Width, unsigned int Height, std::vector<unsigned char>& Data, std::vector<unsigned char>& Dest, TextureToGo* TexToGo);
}