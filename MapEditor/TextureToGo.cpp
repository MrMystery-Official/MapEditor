#include "TextureToGo.h"
#include "TegraSwizzle.h"
#include <stb/stb_image_write.h>
#include <stb/stb_image.h>

/*
    std::vector<unsigned char>& GetPixles();
    uint16_t& GetWidth();
    uint16_t& GetHeight();
    uint16_t& GetDepth();
    uint16_t& GetFormat();
    uint8_t& GetMipMapCount();
*/

std::vector<unsigned char>& TextureToGo::GetPixels()
{
    return this->m_Pixels;
}

uint16_t& TextureToGo::GetWidth()
{
    return this->m_Width;
}

uint16_t& TextureToGo::GetHeight()
{
    return this->m_Height;
}

uint16_t& TextureToGo::GetDepth()
{
    return this->m_Depth;
}

uint16_t& TextureToGo::GetFormat()
{
    return this->m_Format;
}

uint8_t& TextureToGo::GetMipMapCount()
{
    return this->m_MipMapCount;
}

bool& TextureToGo::IsTransparent()
{
    return this->m_Transparent;
}

bool& TextureToGo::IsFail()
{
    return this->m_Fail;
}

TextureToGo::TextureToGo(std::string Path, std::vector<unsigned char> Bytes)
{
    this->m_Fail = false;
    BinaryVectorReader Reader(Bytes);

    uint16_t HeaderSize = Reader.ReadUInt16(); //Always 80
    uint64_t Version = Reader.ReadUInt16(); //Always 17

    char Magic[4];
    Reader.Read(Magic, 4);
    if (Magic[0] != '6' || Magic[1] != 'P' || Magic[2] != 'K' || Magic[3] != '0')
    {
        std::cerr << "Expected 6PK0, got " << Magic << "!\n";
        return;
    }

    this->m_Width = Reader.ReadUInt16();
    this->m_Height = Reader.ReadUInt16();
    this->m_Depth = Reader.ReadUInt16();
    this->m_MipMapCount = Reader.ReadUInt8();

    Reader.Seek(13, BinaryVectorReader::Position::Current);

    uint8_t Hash[32];
    Reader.Read(reinterpret_cast<char*>(Hash), 32);

    this->m_Format = Reader.ReadUInt16();

    Reader.Seek(HeaderSize, BinaryVectorReader::Position::Begin);

    std::vector<TextureToGo::SurfaceInfo> Surfaces(this->m_MipMapCount * this->m_Depth);
    for (int i = 0; i < this->m_MipMapCount * this->m_Depth; i++)
    {
        Surfaces[i].ArrayLevel = Reader.ReadUInt16();
        Surfaces[i].MipMapLevel = Reader.ReadUInt8();
        Reader.Seek(1, BinaryVectorReader::Position::Current); //Always 1 - SurfaceCount
    }

    for (int i = 0; i < this->m_MipMapCount * this->m_Depth; i++)
    {
        Surfaces[i].Size = Reader.ReadUInt32();
        Reader.Seek(4, BinaryVectorReader::Position::Current); //Always 6
    }

    for (int j = 0; j < this->m_Depth; j++) {

        std::vector<unsigned char> RawImageData(Surfaces[j].Size);
        std::vector<unsigned char> Result;

        for (int i = 0; i < Surfaces[j].Size; i++)
        {
            RawImageData[i] = Reader.ReadUInt8();
        }

        Result.resize(ZSTD_getFrameContentSize(RawImageData.data(), RawImageData.size()));
        ZSTD_DCtx* const DCtx = ZSTD_createDCtx();
        ZSTD_decompressDCtx(DCtx, (void*)Result.data(), Result.size(), RawImageData.data(), RawImageData.size());
        ZSTD_freeDCtx(DCtx);

        Result = TegraSwizzle::GetDirectImageData(this, Result, this->m_Format, this->m_Width, this->m_Height, j);

        this->m_Pixels.insert(this->m_Pixels.end(), Result.begin(), Result.end());
    }

    std::vector<unsigned char> Input = this->m_Pixels;

    if (this->DecompressFunction != nullptr && Input.size() > 0)
    {
        this->m_Pixels.clear();
        DecompressFunction(this->m_Width, this->m_Height, Input, this->m_Pixels, this);

        int Length = 0;
        unsigned char* PNG = stbi_write_png_to_mem_forward(this->m_Pixels.data(), this->m_Width * 4, this->m_Width, this->m_Height, 4, &Length);

        std::vector<unsigned char> FileData(Length + 1);
        FileData[0] = this->m_Transparent;
        std::copy(PNG, PNG + Length, FileData.data() + 1);

        std::ofstream OutputFile(Config::GetWorkingDirFile("Cache/" + Path.substr(Path.find_last_of("/\\") + 1) + ".epng"), std::ios::binary);
        OutputFile.write((char*)FileData.data(), FileData.size());
        OutputFile.close();

        free(PNG);

        //stbi_write_png(Config::GetWorkingDirFile("Cache/" + Path.substr(Path.find_last_of("/\\") + 1) + (this->m_Transparent ? "_Trans" : "") + ".png").c_str(), this->m_Width, this->m_Height, 4, this->m_Pixels.data(), this->m_Width * 4);
    }
    else
    {
        this->m_Pixels.resize(this->m_Width * this->m_Height * 4);
        for (int i = 0; i < this->m_Width * this->m_Height / 2; i++)
        {
            //Red-black texture indicating that the texture format is unsupported
            this->m_Pixels[i * 8] = 255;
            this->m_Pixels[i * 8 + 1] = 0;
            this->m_Pixels[i * 8 + 2] = 0;
            this->m_Pixels[i * 8 + 3] = 255;

            this->m_Pixels[i * 8 + 4] = 0;
            this->m_Pixels[i * 8 + 5] = 0;
            this->m_Pixels[i * 8 + 6] = 0;
            this->m_Pixels[i * 8 + 7] = 255;
        }
    }
}

TextureToGo::TextureToGo(std::string Path)
{
    if (Util::FileExists(Config::GetWorkingDirFile("Cache/" + Path.substr(Path.find_last_of("/\\") + 1) + ".epng")))
    {
        std::ifstream File(Config::GetWorkingDirFile("Cache/" + Path.substr(Path.find_last_of("/\\") + 1) + ".epng"), std::ios::binary);
        File.seekg(0, std::ios_base::end);

        std::streampos FileSize = File.tellg();

        std::vector<unsigned char> Bytes(FileSize);

        File.seekg(0, std::ios_base::beg);
        File.read(reinterpret_cast<char*>(Bytes.data()), FileSize);

        File.close();

        int Width, Height, Components;
        unsigned char* Image = (unsigned char*)stbi_load_from_memory(Bytes.data() + 1, Bytes.size() - 1, &Width, &Height, &Components, STBI_rgb_alpha);
        
        this->m_Pixels.assign(Image, Image + Width * Height * 4);

        this->m_Width = Width;
        this->m_Height = Height;
        this->m_Transparent = Bytes[0];
        stbi_image_free(Image);
        return;
    }

    std::ifstream File(Path, std::ios::binary);

    if (!File.eof() && !File.fail())
    {
        File.seekg(0, std::ios_base::end);
        std::streampos FileSize = File.tellg();

        std::vector<unsigned char> Bytes(FileSize);

        File.seekg(0, std::ios_base::beg);
        File.read(reinterpret_cast<char*>(Bytes.data()), FileSize);

        File.close();

        this->TextureToGo::TextureToGo(Path, Bytes);
    }
    else
    {
        std::cerr << "Could not open file \"" << Path << "\"!\n";
    }
}

/*
bool ActorModelLibrary::IsModelLoaded(std::string ModelName)
{
    return ActorModelLibrary::Models.count(ModelName);
}

BfresFile* ActorModelLibrary::GetModel(std::string ModelName)
{
    std::cout << ModelName << std::endl;

    if (!ActorModelLibrary::IsModelLoaded(ModelName))
        ActorModelLibrary::Models.insert({ ModelName, BfresFile(Config::GetBfresFile(ModelName + ".bfres")) });
    return &ActorModelLibrary::Models[ModelName];
}

std::map<std::string, BfresFile>& ActorModelLibrary::GetModels()
{
    return ActorModelLibrary::Models;
}
    */

std::map<std::string, TextureToGo> TextureToGoLibrary::Textures;

bool TextureToGoLibrary::IsTextureLoaded(std::string Name)
{
    return TextureToGoLibrary::Textures.count(Name);
}

TextureToGo* TextureToGoLibrary::GetTexture(std::string Name)
{
    if (!TextureToGoLibrary::IsTextureLoaded(Name))
        TextureToGoLibrary::Textures.insert({ Name, Config::GetTextureFile(Name)});
    return &TextureToGoLibrary::Textures[Name];
}

std::map<std::string, TextureToGo>& TextureToGoLibrary::GetTextures()
{
    return TextureToGoLibrary::Textures;
}