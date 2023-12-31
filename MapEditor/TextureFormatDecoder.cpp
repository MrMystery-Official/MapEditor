#include "TextureFormatDecoder.h"

#include "astc_decomp.h"

unsigned long PackRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	return ((r << 24) | (g << 16) | (b << 8) | a);
}

void DecompressBlockDXT1(unsigned long x, unsigned long y, unsigned long width, const unsigned char* blockStorage, std::vector<unsigned char>& Image, TextureToGo* TexToGo)
{
	unsigned short color0 = *reinterpret_cast<const unsigned short*>(blockStorage);
	unsigned short color1 = *reinterpret_cast<const unsigned short*>(blockStorage + 2);

	unsigned long temp;

	temp = (color0 >> 11) * 255 + 16;
	unsigned char r0 = (unsigned char)((temp / 32 + temp) / 32);
	temp = ((color0 & 0x07E0) >> 5) * 255 + 32;
	unsigned char g0 = (unsigned char)((temp / 64 + temp) / 64);
	temp = (color0 & 0x001F) * 255 + 16;
	unsigned char b0 = (unsigned char)((temp / 32 + temp) / 32);

	temp = (color1 >> 11) * 255 + 16;
	unsigned char r1 = (unsigned char)((temp / 32 + temp) / 32);
	temp = ((color1 & 0x07E0) >> 5) * 255 + 32;
	unsigned char g1 = (unsigned char)((temp / 64 + temp) / 64);
	temp = (color1 & 0x001F) * 255 + 16;
	unsigned char b1 = (unsigned char)((temp / 32 + temp) / 32);

	unsigned long code = *reinterpret_cast<const unsigned long*>(blockStorage + 4);

	for (int j = 0; j < 4; j++)
	{
		for (int i = 0; i < 4; i++)
		{
			unsigned char FinalColorR = 0;
			unsigned char FinalColorG = 0;
			unsigned char FinalColorB = 0;
			unsigned char FinalColorA = 255;
			unsigned char positionCode = (code >> 2 * (4 * j + i)) & 0x03;

			if (color0 > color1)
			{
				switch (positionCode)
				{
				case 0:
					FinalColorR = r0;
					FinalColorG = g0;
					FinalColorB = b0;
					break;
				case 1:
					FinalColorR = r1;
					FinalColorG = g1;
					FinalColorB = b1;
					break;
				case 2:
					FinalColorR = (2 * r0 + r1) / 3;
					FinalColorG = (2 * g0 + g1) / 3;
					FinalColorB = (2 * b0 + b1) / 3;
					break;
				case 3:
					FinalColorR = (r0 + 2 * r1) / 3;
					FinalColorG = (g0 + 2 * g1) / 3;
					FinalColorB = (b0 + 2 * b1) / 3;
					break;
				}
			}
			else
			{
				switch (positionCode)
				{
				case 0:
					FinalColorR = r0;
					FinalColorG = g0;
					FinalColorB = b0;
					break;
				case 1:
					FinalColorR = r1;
					FinalColorG = g1;
					FinalColorB = b1;
					break;
				case 2:
					FinalColorR = (r0 + r1) / 2;
					FinalColorG = (g0 + g1) / 2;
					FinalColorB = (b0 + b1) / 2;
					break;
				case 3:
					FinalColorA = 0;
					TexToGo->IsTransparent() = true;
					break;
				}
			}

			if (x + i < width) {
				Image[((y + j) * width + (x + i)) * 4] = FinalColorR;
				Image[((y + j) * width + (x + i)) * 4 + 1] = FinalColorG;
				Image[((y + j) * width + (x + i)) * 4 + 2] = FinalColorB;
				Image[((y + j) * width + (x + i)) * 4 + 3] = FinalColorA;
			}
		}
	}
}

void DecompressBlockDXT5(unsigned long x, unsigned long y, unsigned long width, const unsigned char* blockStorage, std::vector<unsigned char>& Image, TextureToGo* TexToGo)
{
	unsigned char alpha0 = *reinterpret_cast<const unsigned char*>(blockStorage);
	unsigned char alpha1 = *reinterpret_cast<const unsigned char*>(blockStorage + 1);

	const unsigned char* bits = blockStorage + 2;
	unsigned long alphaCode1 = bits[2] | (bits[3] << 8) | (bits[4] << 16) | (bits[5] << 24);
	unsigned short alphaCode2 = bits[0] | (bits[1] << 8);

	unsigned short color0 = *reinterpret_cast<const unsigned short*>(blockStorage + 8);
	unsigned short color1 = *reinterpret_cast<const unsigned short*>(blockStorage + 10);

	unsigned long temp;

	temp = (color0 >> 11) * 255 + 16;
	unsigned char r0 = (unsigned char)((temp / 32 + temp) / 32);
	temp = ((color0 & 0x07E0) >> 5) * 255 + 32;
	unsigned char g0 = (unsigned char)((temp / 64 + temp) / 64);
	temp = (color0 & 0x001F) * 255 + 16;
	unsigned char b0 = (unsigned char)((temp / 32 + temp) / 32);

	temp = (color1 >> 11) * 255 + 16;
	unsigned char r1 = (unsigned char)((temp / 32 + temp) / 32);
	temp = ((color1 & 0x07E0) >> 5) * 255 + 32;
	unsigned char g1 = (unsigned char)((temp / 64 + temp) / 64);
	temp = (color1 & 0x001F) * 255 + 16;
	unsigned char b1 = (unsigned char)((temp / 32 + temp) / 32);

	unsigned long code = *reinterpret_cast<const unsigned long*>(blockStorage + 12);

	for (int j = 0; j < 4; j++)
	{
		for (int i = 0; i < 4; i++)
		{
			int alphaCodeIndex = 3 * (4 * j + i);
			int alphaCode;

			if (alphaCodeIndex <= 12)
			{
				alphaCode = (alphaCode2 >> alphaCodeIndex) & 0x07;
			}
			else if (alphaCodeIndex == 15)
			{
				alphaCode = (alphaCode2 >> 15) | ((alphaCode1 << 1) & 0x06);
			}
			else // alphaCodeIndex >= 18 && alphaCodeIndex <= 45
			{
				alphaCode = (alphaCode1 >> (alphaCodeIndex - 16)) & 0x07;
			}

			unsigned char finalAlpha;
			if (alphaCode == 0)
			{
				finalAlpha = alpha0;
			}
			else if (alphaCode == 1)
			{
				finalAlpha = alpha1;
			}
			else
			{
				if (alpha0 > alpha1)
				{
					finalAlpha = ((8 - alphaCode) * alpha0 + (alphaCode - 1) * alpha1) / 7;
				}
				else
				{
					if (alphaCode == 6)
						finalAlpha = 0;
					else if (alphaCode == 7)
						finalAlpha = 255;
					else
						finalAlpha = ((6 - alphaCode) * alpha0 + (alphaCode - 1) * alpha1) / 5;
				}
			}

			unsigned char colorCode = (code >> 2 * (4 * j + i)) & 0x03;

			unsigned char FinalColorR = 0;
			unsigned char FinalColorG = 0;
			unsigned char FinalColorB = 0;
			switch (colorCode)
			{
			case 0:
				FinalColorR = r0;
				FinalColorG = g0;
				FinalColorB = b0;
				break;
			case 1:
				FinalColorR = r1;
				FinalColorG = g1;
				FinalColorB = b1;
				break;
			case 2:
				FinalColorR = (2 * r0 + r1) / 3;
				FinalColorG = (2 * g0 + g1) / 3;
				FinalColorB = (2 * b0 + b1) / 3;
				break;
			case 3:
				FinalColorR = (r0 + 2 * r1) / 3;
				FinalColorG = (g0 + 2 * g1) / 3;
				FinalColorB = (b0 + 2 * b1) / 3;
				break;
			}

			if (x + i < width) {
				Image[((y + j) * width + (x + i)) * 4] = FinalColorR;
				Image[((y + j) * width + (x + i)) * 4 + 1] = FinalColorG;
				Image[((y + j) * width + (x + i)) * 4 + 2] = FinalColorB;
				Image[((y + j) * width + (x + i)) * 4 + 3] = finalAlpha;
				if (finalAlpha != 255)
					TexToGo->IsTransparent() = true;
			}
		}
	}
}

void TextureFormatDecoder::DecodeBC1(unsigned int Width, unsigned int Height, std::vector<unsigned char>& Data, std::vector<unsigned char>& Dest, TextureToGo* TexToGo)
{
	Dest.resize(Width * Height * 10);

	unsigned long BlockCountX = (Width + 3) / 4;
	unsigned long BlockCountY = (Height + 3) / 4;
	unsigned long BlockWidth = (Width < 4) ? Width : 4;
	unsigned long BlockHeight = (Height < 4) ? Height : 4;
	unsigned char* BlockStorage = Data.data();

	for (unsigned long j = 0; j < BlockCountY; j++)
	{
		for (unsigned long i = 0; i < BlockCountX; i++) DecompressBlockDXT1(i * 4, j * 4, Width, BlockStorage + i * 8, Dest, TexToGo);
		BlockStorage += BlockCountX * 8;
	}

	Dest.resize(Width * Height * 4);
}

void TextureFormatDecoder::DecodeBC3SRG(unsigned int Width, unsigned int Height, std::vector<unsigned char>& Data, std::vector<unsigned char>& Dest, TextureToGo* TexToGo)
{
	Dest.resize(Width * Height * 5);

	unsigned long BlockCountX = (Width + 3) / 4;
	unsigned long BlockCountY = (Height + 3) / 4;
	unsigned long BlockWidth = (Width < 4) ? Width : 4;
	unsigned long BlockHeight = (Height < 4) ? Height : 4;
	unsigned char* BlockStorage = Data.data();

	for (unsigned long j = 0; j < BlockCountY; j++)
	{
		for (unsigned long i = 0; i < BlockCountX; i++) DecompressBlockDXT5(i * 4, j * 4, Width, BlockStorage + i * 16, Dest, TexToGo);
		BlockStorage += BlockCountX * 16;
	}

	Dest.resize(Width * Height * 4);
}

void TextureFormatDecoder::DecodeASTC8x8UNorm(unsigned int Width, unsigned int Height, std::vector<unsigned char>& Data, std::vector<unsigned char>& Dest, TextureToGo* TexToGo)
{
	// 1 decoded block contains 64 pixels

	// Calculate the number of blocks in the image
	int NumBlocks = (Width + 7) / 8 * (Height + 7) / 8;
	Dest.resize(NumBlocks * 64 * 4); // Allocate space for all decoded pixels in RGB format, 4 bytes per pixel

	unsigned char* DestPtr = Dest.data();

	// Split up the data into blocks
	for (int i = 0; i < NumBlocks; i++)
	{
		basisu::astc::decompress((unsigned char*)DestPtr + i * 64 * 4, &Data[i * 16], false, 8, 8); // Pass the data pointer directly to the basisu::astc::decompress function
	}

	Data.resize(Width * Height * 4); //4 bytes per pixel, RGBA
}