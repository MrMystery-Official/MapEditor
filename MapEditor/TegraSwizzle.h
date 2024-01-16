#pragma once

#include <cmath>
#include "TextureFormatDecoder.h"
#include "TextureToGo.h"

namespace TegraSwizzle
{
	typedef struct FormatInfo
	{
		uint32_t BytesPerPixel;
		uint32_t BlockWidth;
		uint32_t BlockHeight;
		void (*DecompressFunction)(unsigned int, unsigned int, std::vector<unsigned char>&, std::vector<unsigned char>&, TextureToGo* TexToGo);
	};

	typedef struct TegraSwizzleFormats
	{
		FormatInfo BC1_UNORM;
		FormatInfo BC3_UNORM_SRGB;
		FormatInfo BC4_UNORM;
		FormatInfo ASTC_8x8_UNORM;
		FormatInfo ASTC_4x4_UNORM;
		FormatInfo UNSUPPORTED;
	};

	FormatInfo GetFormatInfo(uint16_t FormatId);
	uint32_t Max(uint32_t a, uint32_t b);
	uint32_t DIV_ROUND_UP(uint32_t n, uint32_t d);
	uint32_t Pow2RoundUp(uint32_t x);
	uint32_t RoundUp(uint32_t x, uint32_t y);
	uint32_t GetBlockHeight(uint32_t Height);
	uint32_t GetAddrBlockLinear(uint32_t x, uint32_t y, uint32_t Width, uint32_t BytesPerPixel, uint32_t BaseAddress, uint32_t BlockHeight);
	std::vector<unsigned char> Deswizzle(uint32_t Width, uint32_t Height, uint32_t Depth, uint32_t BlockWidth, uint32_t FormatBlockHeight, uint32_t BlockDepth, int32_t RoundPitch, uint32_t BytesPerPixel, uint32_t TileMode, int32_t SizeRange, std::vector<unsigned char>& ImageData);
	std::vector<unsigned char> GetDirectImageData(TextureToGo* Texture, std::vector<unsigned char> Pixels, uint16_t Format, uint16_t Width, uint16_t Height, uint16_t Depth, int32_t Target = 1, bool LinearTileMode = false);
	std::vector<unsigned char> GetDirectImageData(TextureToGo* Texture, int32_t Target = 1, bool LinearTileMode = false);
	std::vector<unsigned char> GetDirectImageData(TextureToGo* TexToGo, std::vector<unsigned char> Pixels, uint16_t TextureFormat, uint16_t TextureWidth, uint16_t TextureHeight, uint16_t TextureDepth, uint32_t BlockHeightLog2, int32_t Target, bool LinearTileMode);
};