#include "TegraSwizzle.h"

const TegraSwizzle::TegraSwizzleFormats Formats =
{
	.BC1_UNORM = {8, 4, 4, TextureFormatDecoder::DecodeBC1},
	.BC3_UNORM_SRGB = {16, 4, 4, TextureFormatDecoder::DecodeBC3SRG},
	.BC4_UNORM = {8, 4, 4, TextureFormatDecoder::DecodeBC4},
	.ASTC_8x8_UNORM = {16, 8, 8, TextureFormatDecoder::DecodeASTC8x8UNorm}, //0x101
	.ASTC_4x4_UNORM = {16, 4, 4, TextureFormatDecoder::DecodeASTC4x4UNorm}, //0x102, currently not fully implemented
	.UNSUPPORTED = {0, 0, 0, nullptr}
};

TegraSwizzle::FormatInfo TegraSwizzle::GetFormatInfo(uint16_t FormatId)
{
	switch (FormatId)
	{
	case 0x202:
		return Formats.BC1_UNORM;
	case 0x302:
		return Formats.BC1_UNORM;
	case 0x505:
		return Formats.BC3_UNORM_SRGB;
	case 0x606:
		return Formats.BC4_UNORM;
	case 0x602:
		return Formats.BC4_UNORM;
	case 0x101:
		return Formats.ASTC_8x8_UNORM;
	default:
		std::cout << "UNKNOWN TEXTURE FORMAT " << FormatId << "!\n";
		return Formats.UNSUPPORTED;
	}
}

uint32_t TegraSwizzle::Max(uint32_t a, uint32_t b)
{
	return a < b ? b : a;
}

uint32_t TegraSwizzle::DIV_ROUND_UP(uint32_t n, uint32_t d)
{
	return (n + d - 1) / d;
}

uint32_t TegraSwizzle::Pow2RoundUp(uint32_t x)
{
	x -= 1;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x + 1;
}

uint32_t TegraSwizzle::RoundUp(uint32_t x, uint32_t y)
{
	return ((x - 1) | (y - 1)) + 1;
}

uint32_t TegraSwizzle::GetBlockHeight(uint32_t Height)
{
	uint32_t BlockHeight = Pow2RoundUp(Height / 8);
	if (BlockHeight > 16)
		BlockHeight = 16;

	return BlockHeight;
}

uint32_t TegraSwizzle::GetAddrBlockLinear(uint32_t x, uint32_t y, uint32_t Width, uint32_t BytesPerPixel, uint32_t BaseAddress, uint32_t BlockHeight)
{
	uint32_t ImageWidthInGobs = DIV_ROUND_UP(Width * BytesPerPixel, 64);
	uint32_t GOBAddress = (BaseAddress
		+ (y / (8 * BlockHeight)) * 512 * BlockHeight * ImageWidthInGobs
		+ (x * BytesPerPixel / 64) * 512 * BlockHeight
		+ (y % (8 * BlockHeight) / 8) * 512);

	x *= BytesPerPixel;

	/* What the fuck is this...? */
	uint32_t Address = (GOBAddress + ((x % 64) / 32) * 256 + ((y % 8) / 2) * 64
		+ ((x % 32) / 16) * 32 + (y % 2) * 16 + (x % 16));

	return Address;
}

std::vector<unsigned char> TegraSwizzle::Deswizzle(uint32_t Width, uint32_t Height, uint32_t Depth, uint32_t BlockWidth, uint32_t FormatBlockHeight, uint32_t BlockDepth, int32_t RoundPitch, uint32_t BytesPerPixel, uint32_t TileMode, int32_t SizeRange, std::vector<unsigned char>& ImageData)
{
	uint32_t BlockHeight = (uint32_t)(1 << SizeRange);
	Width = DIV_ROUND_UP(Width, BlockWidth);
	Height = DIV_ROUND_UP(Height, FormatBlockHeight);
	Depth = DIV_ROUND_UP(Depth, BlockDepth);

	uint32_t Pitch = 0;
	uint32_t SurfaceSize = 0;
	if (TileMode == 1)
	{
		Pitch = Width * BytesPerPixel;
		if (RoundPitch == 1)
			Pitch = RoundUp(Pitch, 32);

		SurfaceSize = Pitch * Height;
	}
	else
	{
		Pitch = RoundUp(Width * BytesPerPixel, 64);
		SurfaceSize = Pitch * RoundUp(Height, BlockHeight * 8);
	}

	std::vector<unsigned char> Result(SurfaceSize);

	for (uint32_t y = 0; y < Height; y++)
	{
		for (uint32_t x = 0; x < Width; x++)
		{
			uint32_t pos;
			uint32_t pos_;

			if (TileMode == 1)
			{
				pos = y * Pitch + x * BytesPerPixel;
			}
			else
			{
				pos = GetAddrBlockLinear(x, y, Width, BytesPerPixel, 0, BlockHeight);
			}

			pos_ = (y * Width + x) * BytesPerPixel;

			if (pos + BytesPerPixel <= SurfaceSize)
			{
				if (ImageData.size() >= pos + BytesPerPixel)
				{
					std::copy(ImageData.begin() + pos, ImageData.begin() + pos + BytesPerPixel, Result.begin() + pos_);
				}
			}
		}
	}

	return Result;
}

std::vector<unsigned char> TegraSwizzle::GetDirectImageData(TextureToGo* Texture, std::vector<unsigned char> Pixels, uint16_t Format, uint16_t Width, uint16_t Height, uint16_t Depth, int32_t Target, bool LinearTileMode)
{
	uint32_t BlockHeight = GetBlockHeight(DIV_ROUND_UP(Height, 4));

	if (BlockHeight == 0) BlockHeight = 4;

	return GetDirectImageData(Texture, Pixels, Format, Width, Height, Depth, std::log10(BlockHeight) / std::log10(2), Target, LinearTileMode);
}

std::vector<unsigned char> TegraSwizzle::GetDirectImageData(TextureToGo* Texture, int32_t Target, bool LinearTileMode)
{
	uint32_t BlockHeight = GetBlockHeight(DIV_ROUND_UP(Texture->GetHeight(), 4));

	if (BlockHeight == 0) BlockHeight = 4;

	return GetDirectImageData(Texture, Texture->GetPixels(), Texture->GetFormat(), Texture->GetWidth(), Texture->GetHeight(), Texture->GetDepth(), std::log10(BlockHeight) / std::log10(2), Target, LinearTileMode);
}

std::vector<unsigned char> TegraSwizzle::GetDirectImageData(TextureToGo* TexToGo, std::vector<unsigned char> Pixels, uint16_t TextureFormat, uint16_t TextureWidth, uint16_t TextureHeight, uint16_t TextureDepth, uint32_t BlockHeightLog2, int32_t Target, bool LinearTileMode)
{

	TegraSwizzle::FormatInfo Format = GetFormatInfo(TextureFormat);
	if (Format.DecompressFunction == nullptr)
	{
		return std::vector<unsigned char>(0);
	}
	TexToGo->DecompressFunction = Format.DecompressFunction;

	uint32_t BytesPerPixel = Format.BytesPerPixel;
	uint32_t BlockWidth = Format.BlockWidth;
	uint32_t FormatBlockHeight = Format.BlockHeight;
	uint32_t BlockDepth = 1;

	uint32_t BlockHeight = GetBlockHeight(DIV_ROUND_UP(TextureHeight, FormatBlockHeight)); //4=BlockHeight of BC1

	uint32_t DataAlignment = 512;
	uint32_t TileMode = LinearTileMode ? 1 : 0;

	int32_t LinesPerBlockHeight = (1 << (int32_t)BlockHeightLog2) * 8;

	uint32_t ArrayOffset = 0;

	int32_t BlockHeightShift = 0;
	uint32_t Width = Max(1, TextureWidth);
	uint32_t Height = Max(1, TextureHeight);
	uint32_t Depth = Max(1, TextureDepth);

	uint32_t Size = DIV_ROUND_UP(Width, BlockWidth) * DIV_ROUND_UP(Height, FormatBlockHeight) * BytesPerPixel;
	if (Pow2RoundUp(DIV_ROUND_UP(Height, BlockWidth)) < LinesPerBlockHeight)
		BlockHeightShift++;

	uint32_t Width__ = DIV_ROUND_UP(Width, BlockWidth);
	uint32_t Height__ = DIV_ROUND_UP(Height, FormatBlockHeight);

	std::vector<unsigned char> AlignedData = Deswizzle(Width, Height, Depth, BlockWidth, FormatBlockHeight, BlockDepth, Target, BytesPerPixel, TileMode, Max(0, BlockHeightLog2 - BlockHeightShift), Pixels);
	AlignedData.resize(Size);
	return AlignedData;
}