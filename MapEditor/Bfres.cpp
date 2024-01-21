#include "Bfres.h"

std::map<TextureToGo*, Texture> GLTextureLibrary::Textures;

bool GLTextureLibrary::IsTextureLoaded(TextureToGo* TexToGo)
{
    return GLTextureLibrary::Textures.count(TexToGo);
}
Texture* GLTextureLibrary::GetTexture(TextureToGo* TexToGo)
{
    if (!GLTextureLibrary::IsTextureLoaded(TexToGo))
        GLTextureLibrary::Textures.insert({ TexToGo, Texture(TexToGo, GL_RGBA, GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE) });
    return &GLTextureLibrary::Textures[TexToGo];
}
std::map<TextureToGo*, Texture>& GLTextureLibrary::GetTextures()
{
    return GLTextureLibrary::Textures;
}

float MaxBfres(float a, float b) {
    return a > b ? a : b;
}

void BfresFile::GenerateBoundingBox() {
    for (int SubModelIndex = 0; SubModelIndex < this->m_Models[0].Vertices.size(); SubModelIndex++)
    {
        for (float Vertex : this->m_Models[0].Vertices[SubModelIndex])
        {
            this->m_Models[0].BoundingBoxSphereRadius = MaxBfres(this->m_Models[0].BoundingBoxSphereRadius, std::fabs(Vertex));
        }
    }
}

void BfresFile::CreateOpenGLObjects()
{
    /* Creating OpenGL Objects */
    for (BfresFile::Model& Model : this->m_Models)
    {
        for (BfresFile::LOD& LODModel : Model.LODs)
        {
            LODModel.GL_Textures.resize(LODModel.Faces.size());
            LODModel.GL_VAO.resize(LODModel.Faces.size());
            LODModel.GL_VBO.resize(LODModel.Faces.size());
            LODModel.GL_EBO.resize(LODModel.Faces.size());
            int FaceOffset = 0;
            for (int SubModelIndex = 0; SubModelIndex < LODModel.Faces.size(); SubModelIndex++)
            {
                if (Model.Materials[SubModelIndex].Textures.empty())
                {
                    LODModel.Faces.erase(LODModel.Faces.begin() + SubModelIndex);
                    FaceOffset++;
                    continue;
                }
                std::vector<float>* TexCoords = &Model.Materials[SubModelIndex].Textures[0].TexCoordinates;
                if (TexCoords->empty())
                {
                    LODModel.Faces.erase(LODModel.Faces.begin() + SubModelIndex);
                    FaceOffset++;
                    continue;
                }

                LODModel.GL_Textures[SubModelIndex] = GLTextureLibrary::GetTexture(Model.Materials[SubModelIndex].Textures[0].Texture);

                VAO VAO1 = VAO(true);
                VAO1.Bind();

                std::vector<GLfloat> Vertices(Model.Vertices[SubModelIndex].size() / 3 * 5);

                for (int VertexIndex = 0; VertexIndex < Vertices.size() / 5; VertexIndex++)
                {
                    Vertices[VertexIndex * 5] = Model.Vertices[SubModelIndex][VertexIndex * 3];
                    Vertices[VertexIndex * 5 + 1] = Model.Vertices[SubModelIndex][VertexIndex * 3 + 1];
                    Vertices[VertexIndex * 5 + 2] = Model.Vertices[SubModelIndex][VertexIndex * 3 + 2];

                    Vertices[VertexIndex * 5 + 3] = TexCoords->at(VertexIndex * 2);
                    Vertices[VertexIndex * 5 + 4] = TexCoords->at(VertexIndex * 2 + 1);
                }

                // Generates Vertex Buffer Object and links it to vertices
                VBO VBO1(Vertices.data(), sizeof(float) * Vertices.size());
                // Generates Element Buffer Object and links it to indices
                EBO EBO1(LODModel.Faces[SubModelIndex - FaceOffset].data(), sizeof(int) * LODModel.Faces[SubModelIndex - FaceOffset].size());

                // Links VBO attributes such as coordinates and colors to VAO
                VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 5 * sizeof(float), (void*)0);
                VAO1.LinkAttrib(VBO1, 2, 2, GL_FLOAT, 5 * sizeof(float), (void*)(3 * sizeof(float)));
                // Unbind all to prevent accidentally modifying them

                VAO1.Unbind();
                VBO1.Unbind();
                EBO1.Unbind();

                LODModel.GL_VAO[SubModelIndex] = VAO1;
                LODModel.GL_VBO[SubModelIndex] = VBO1;
                LODModel.GL_EBO[SubModelIndex] = EBO1;
            }
        }
    }

    this->GenerateBoundingBox();
}

BfresFile BfresFile::CreateDefaultModel(std::string DefaultName, uint8_t Red, uint8_t Green, uint8_t Blue, uint8_t Alpha)
{
    BfresFile DefaultModel;
    DefaultModel.IsDefaultModel() = true;

    const float Size = 0.5f;

    float Vertices[] =
    {
        -Size, -Size, -Size,
         Size, -Size, -Size,
         Size, -Size,  Size,
        -Size, -Size,  Size,

        // Top face
        -Size,  Size, -Size,
         Size,  Size, -Size,
         Size,  Size,  Size,
        -Size,  Size,  Size,
    };

    unsigned int Indices[] =
    {
        // Bottom face
        0, 1, 2,
        0, 2, 3,

        // Top face
        4, 5, 6,
        4, 6, 7,

        // Front face
        0, 4, 5,
        0, 5, 1,

        // Back face
        3, 7, 6,
        3, 6, 2,

        // Left face
        0, 3, 7,
        0, 7, 4,

        1, 5, 6,
        1, 6, 2,
    };

    float TexCoords[] =
    {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,

        // Top face
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };

    BfresFile::Model Model;
    BfresFile::LOD LODModel;
    LODModel.Faces.push_back(std::vector<unsigned int>(std::begin(Indices), std::end(Indices)));
    Model.Vertices.push_back(std::vector<float>(std::begin(Vertices), std::end(Vertices)));

    BfresFile::Material Material;
    Material.Name = "Mt_DefaultModel";

    BfresFile::BfresTexture Texture;
    Texture.TexCoordinates = std::vector<float>(std::begin(TexCoords), std::end(TexCoords));

    TextureToGo TexToGo;
    TexToGo.IsFail() = false;
    TexToGo.GetPixels() = { Red, Green, Blue, Alpha };
    TexToGo.GetWidth() = 1;
    TexToGo.GetHeight() = 1;
    if(!TextureToGoLibrary::IsTextureLoaded(DefaultName))
        TextureToGoLibrary::Textures.insert({ DefaultName, TexToGo });

    Texture.Texture = TextureToGoLibrary::GetTexture(DefaultName);

    Material.Textures.push_back(Texture);
    Model.Materials.push_back(Material);
    Model.LODs.push_back(LODModel);
    DefaultModel.GetModels().push_back(Model);

    DefaultModel.CreateOpenGLObjects();

    return DefaultModel;
}

float BfresFile::ShortToFloat(uint8_t Byte1, uint8_t Byte2)
{
    uint16_t CombinedValue = (static_cast<uint16_t>(Byte2) << 8) | static_cast<uint16_t>(Byte1);
    if (CombinedValue == 0)
    {
        return 0;
    }
    int32_t BiasedExponent = (CombinedValue >> 10) & 0x1F;
    int32_t Mantissa = CombinedValue & 0x3FF;

    // Reconstruct the half-float value with proper exponent bias
    int32_t RealExponent = BiasedExponent - 15 + 127;
    uint32_t HalfFloatBits = ((CombinedValue & 0x8000) << 16) | (RealExponent << 23) | (Mantissa << 13);
    float Result;
    std::memcpy(&Result, &HalfFloatBits, sizeof(float));
    return Result;
}

float BfresFile::UInt32ToFloat(unsigned char Byte1, unsigned char Byte2, unsigned char Byte3, unsigned char Byte4)
{
    // Combine the four bytes into a uint32_t
    uint32_t combinedValue = (static_cast<uint32_t>(Byte4) << 24) |
        (static_cast<uint32_t>(Byte3) << 16) |
        (static_cast<uint32_t>(Byte2) << 8) |
        static_cast<uint32_t>(Byte1);

    if (combinedValue == 0)
    {
        return 0;
    }

    float ret;
    std::memcpy(&ret, &combinedValue, sizeof(float));
    return ret;
}

uint16_t BfresFile::CombineToUInt16(uint8_t Byte1, uint8_t Byte2)
{
    return (static_cast<uint16_t>(Byte2) << 8) | Byte1;
}

std::string BfresFile::ReadString(BinaryVectorReader& Reader, uint64_t Offset)
{
    Reader.Seek(Offset, BinaryVectorReader::Position::Begin);
    std::string Result;
    char CurrentCharacter = Reader.ReadInt8();
    Result += CurrentCharacter;
    while (CurrentCharacter != 0x00) {
        CurrentCharacter = Reader.ReadInt8();
        Result += CurrentCharacter;
    }
    Result.pop_back();
    return Result;
}

std::vector<BfresFile::Model>& BfresFile::GetModels()
{
    return this->m_Models;
}

bool& BfresFile::IsDefaultModel()
{
    return this->m_DefaultModel;
}

BfresFile::BfresFile(std::string Path, std::vector<unsigned char> Bytes)
{
    BinaryVectorReader Reader(Bytes);

    char Magic[4];
    Reader.Read(Magic, 4);
    if (Magic[0] != 'F' && Magic[1] != 'R' && Magic[2] != 'E' && Magic[3] != 'S')
    {
        std::cerr << "Expected FRES, got " << Magic << "!\n";
        return;
    }

    /* Check if BFRES is for switch, expecting 0x20202020 padding (4 bytes) */
    uint32_t SwitchPadding = Reader.ReadUInt32();
    if (SwitchPadding != 0x20202020)
    {
        std::cerr << "Bfres is not a switch bfres!\n";
        return;
    }

    /* Cheking Version, should be 0.10.0.0 */
    uint8_t VersionNumberD = Reader.ReadUInt8();
    uint8_t VersionNumberC = Reader.ReadUInt8();
    uint8_t VersionNumberB = Reader.ReadUInt8();
    uint8_t VersionNumberA = Reader.ReadUInt8();
    if (VersionNumberA != 0x00 || VersionNumberB != 0x0A || VersionNumberC != 0x00 || VersionNumberD != 0x00)
    {
        std::cerr << "Only BFRES v10 files supported, got BFRES v" << (int)VersionNumberA << "." << (int)VersionNumberB << "." << (int)VersionNumberC << "." << (int)VersionNumberD << "!\n";
        return;
    }

    /* Checking Endian, 0xFEFF indicates that it is Little Endian, what this parser supports */
    uint16_t Endian = Reader.ReadUInt16();
    if (Endian != 0xFEFF)
    {
        std::cerr << "Only BFRES Little Endian supported, got " << Endian << "!\n";
        return;
    }

    Reader.Seek(6, BinaryVectorReader::Position::Current); //Padding

    uint32_t FileAlignment = Reader.ReadUInt32();
    uint32_t RelocationTableOffset = Reader.ReadUInt32();
    uint32_t GlobalBufferOffset = Reader.ReadUInt32();

    Reader.Seek(8, BinaryVectorReader::Position::Current);

    uint64_t FMDLOffset = Reader.ReadUInt64();
    uint64_t FMDLDict = Reader.ReadUInt64();

    Reader.Seek(0xDC, BinaryVectorReader::Position::Begin); //Skipping unnecessary information
    uint16_t FMDLCount = Reader.ReadUInt16();

    this->m_Models.resize(FMDLCount);

    uint32_t DataStart = 0;
    if (GlobalBufferOffset == Bytes.size())
    {
        Reader.Seek(RelocationTableOffset, BinaryVectorReader::Position::Begin);
        Reader.Seek(0x30, BinaryVectorReader::Position::Current);
        DataStart = Reader.ReadUInt32();
    }
    else
    {
        DataStart = GlobalBufferOffset + 288;
    }

    Reader.Seek(FMDLOffset, BinaryVectorReader::Position::Begin);
    for (uint16_t FMDLIndex = 0; FMDLIndex < FMDLCount; FMDLIndex++)
    {
        uint64_t FMDLBaseOffset = Reader.GetPosition();
        Reader.Seek(4, BinaryVectorReader::Position::Current); //Skipping magic, should be FMDL

        Reader.Seek(FMDLBaseOffset + 32, BinaryVectorReader::Position::Begin);
        uint64_t FVTXOffset = Reader.ReadUInt64();
        uint64_t FSHPOffset = Reader.ReadUInt64();
        uint64_t FMATValuesOffset = Reader.ReadUInt64();
        uint64_t FMATDictOffset = Reader.ReadUInt64();

        Reader.Seek(FMDLBaseOffset + 104, BinaryVectorReader::Position::Begin);
        uint16_t FVTXCount = Reader.ReadUInt16();
        uint16_t FSHPCount = Reader.ReadUInt16();
        uint16_t FMATCount = Reader.ReadUInt16();

        this->m_Models[FMDLIndex].Materials.resize(FSHPCount);

        std::vector<Material> Materials(FMATCount);

        //Materials
        Reader.Seek(FMATDictOffset, BinaryVectorReader::Position::Begin);
        for (int FMATIndex = 0; FMATIndex < FMATCount; FMATIndex++)
        {
            uint64_t FMATBaseOffset = Reader.GetPosition();
            Reader.Seek(8, BinaryVectorReader::Position::Current);

            Materials[FMATIndex].Name = this->ReadString(Reader, Reader.ReadUInt64() + 2);

            Reader.Seek(FMATBaseOffset + 32, BinaryVectorReader::Position::Begin);
            uint64_t TextureArrayOffset = Reader.ReadUInt64();
            uint64_t TextureNameArray = Reader.ReadUInt64();
            uint64_t SamplerArrayOffset = Reader.ReadUInt64();

            uint64_t SamplerValuesOffset = Reader.ReadUInt64();
            uint64_t SamplerOffset = Reader.ReadUInt64();

            Reader.Seek(FMATBaseOffset + 162, BinaryVectorReader::Position::Begin);
            uint8_t SamplerCount = Reader.ReadUInt8();
            uint8_t TextureCount = Reader.ReadUInt8();

            Reader.Seek(TextureArrayOffset, BinaryVectorReader::Position::Begin);

            //std::vector<std::string> NrmTextureNames;

            std::vector<TextureToGo*> LocalTextures;
            std::vector<TextureToGo*> LocalTransparentTextures;

            if (Path.find("WaterPlane") != std::string::npos)
            {
                LocalTextures.push_back(TextureToGoLibrary::GetTexture("TerraWater01_Alb.txtg"));
                goto TextureInterpreter;
            }

            for (int i = 0; i < TextureCount; i++)
            {
                std::string TextureName = this->ReadString(Reader, Reader.ReadUInt64() + 2);

                Reader.Seek(SamplerValuesOffset + 32 + i * 16, BinaryVectorReader::Position::Begin);
                Reader.Seek(Reader.ReadUInt64(), BinaryVectorReader::Position::Begin);

                uint16_t NameLength = Reader.ReadUInt16();
                std::string Sampler = "";
                for (int j = 0; j < NameLength; j++) { //-1 to remove the index, normally textures are not _a, they are _a0, _a1, _a2 ... _an. Same for Nrm...
                    Sampler += (char)Reader.ReadInt8();
                }

                if (Sampler.rfind("_a", 0) == 0 || Sampler.rfind("_gn0", 0) == 0) Sampler.pop_back();

                if (Sampler == "_a" || Sampler == "tma" || Sampler == "_gn")
                {
                    TextureToGo* TexToGo = TextureToGoLibrary::GetTexture(TextureName + ".txtg");
                    if (TexToGo->IsTransparent())
                    {
                        LocalTransparentTextures.push_back(TexToGo);
                    }
                    else
                    {
                        LocalTextures.push_back(TexToGo);
                    }
                }
                //else if (Sampler == "_n") NrmTextureNames.push_back(TextureName);

                Reader.Seek(TextureArrayOffset + (i + 1) * 8, BinaryVectorReader::Position::Begin);
            }

            TextureInterpreter:

            int LocalTextureCount = LocalTextures.size();
            int LocalTransparentTextureCount = LocalTransparentTextures.size();
            Materials[FMATIndex].Textures.resize(LocalTextureCount + LocalTransparentTextureCount);


            for (int i = 0; i < (LocalTextureCount + LocalTransparentTextureCount); i++)
            {
                BfresFile::BfresTexture MaterialTexture;
                MaterialTexture.Texture = i < LocalTextureCount ? LocalTextures[i] : LocalTransparentTextures[i - LocalTextureCount];
                if (i >= LocalTextureCount)
                {
                    Materials[FMATIndex].IsTransparent = true;
                }
                Materials[FMATIndex].Textures[i] = MaterialTexture;
            }

            if (Materials[FMATIndex].Textures.empty())
            {
                Reader.Seek(TextureArrayOffset, BinaryVectorReader::Position::Begin);

                for (int i = 0; i < TextureCount; i++)
                {
                    std::string TextureName = this->ReadString(Reader, Reader.ReadUInt64() + 2);

                    Reader.Seek(SamplerValuesOffset + 32 + i * 16, BinaryVectorReader::Position::Begin);
                    Reader.Seek(Reader.ReadUInt64(), BinaryVectorReader::Position::Begin);

                    uint16_t NameLength = Reader.ReadUInt16();
                    std::string Sampler = "";
                    for (int j = 0; j < NameLength; j++) {
                        Sampler += (char)Reader.ReadInt8();
                    }

                    BfresFile::BfresTexture MaterialTexture;
                    MaterialTexture.Texture = TextureToGoLibrary::GetTexture(TextureName + ".txtg");
                    Materials[FMATIndex].Textures.push_back(MaterialTexture);
                    Materials[FMATIndex].IsTransparent = MaterialTexture.Texture->IsTransparent();

                    Reader.Seek(TextureArrayOffset + (i + 1) * 8, BinaryVectorReader::Position::Begin);
                }
            }

            //LocalTextures[FMATIndex] = TextureToGo(TexToGoPath + "/" + AlbTextureNames[AlbTextureNames.size() - 1] + ".txtg");
            //this->m_Models[FMDLIndex].Textures[FMATIndex].Normal = TextureToGo(TexToGoPath + "/" + NrmTextureNames[AlbTextureNames.size() - 1] + ".txtg");

            Reader.Seek(FMATBaseOffset + 176, BinaryVectorReader::Position::Begin);
        }

        //Parsing FSHP
        Reader.Seek(FSHPOffset, BinaryVectorReader::Position::Begin);
        for (int FSHPIndex = 0; FSHPIndex < FSHPCount; FSHPIndex++)
        {
            uint64_t FSHPBaseOffset = Reader.GetPosition();
            Reader.Seek(24, BinaryVectorReader::Position::Current);
            uint64_t MeshArrayOffset = Reader.ReadUInt64();
            uint64_t LODModelOffset = Reader.ReadUInt64();
            Reader.Seek(24, BinaryVectorReader::Position::Current);
            uint64_t BoundingBoxOffset = Reader.ReadUInt64();
            uint64_t RadiusOffset = Reader.ReadUInt64();
            int64_t UserPointer = Reader.ReadInt64();

            Reader.Seek(FSHPBaseOffset + 82, BinaryVectorReader::Position::Begin);
            uint16_t MaterialIndex = Reader.ReadUInt16();

            Reader.Seek(7, BinaryVectorReader::Position::Current);
            uint8_t NumLODs = Reader.ReadUInt8();

            Reader.Seek(MeshArrayOffset, BinaryVectorReader::Position::Begin);
            uint64_t MeshBaseOffset = Reader.GetPosition();
            this->m_Models[FMDLIndex].LODs.resize(NumLODs);

            for (int LODIndex = 0; LODIndex < NumLODs; LODIndex++)
            {
                this->m_Models[FMDLIndex].LODs[LODIndex].Faces.resize(FSHPCount);
                    uint64_t SubMeshArrayOffset = Reader.ReadUInt64();
                    Reader.Seek(24, BinaryVectorReader::Position::Current);
                    uint32_t FaceBufferOffset = Reader.ReadUInt32();
                    Reader.Seek(4, BinaryVectorReader::Position::Current);
                    uint32_t FaceType = Reader.ReadUInt32();
                    uint32_t FaceCount = Reader.ReadUInt32();
                    uint32_t VertexSkip = Reader.ReadUInt32();
                    //uint16_t NumSubMeshes = Reader.ReadUInt16();

                    Reader.Seek(DataStart + FaceBufferOffset, BinaryVectorReader::Position::Begin);

                    this->m_Models[FMDLIndex].LODs[LODIndex].Faces[FSHPIndex].resize(FaceCount);
                    if (FaceType == 1)
                    {
                        for (uint32_t FaceIndex = 0; FaceIndex < FaceCount; FaceIndex++)
                        {
                            this->m_Models[FMDLIndex].LODs[LODIndex].Faces[FSHPIndex][FaceIndex] = VertexSkip + Reader.ReadUInt16();
                        }
                    }
                    else if(FaceType == 2)
                    {
                        for (uint32_t FaceIndex = 0; FaceIndex < FaceCount; FaceIndex++)
                        {
                            this->m_Models[FMDLIndex].LODs[LODIndex].Faces[FSHPIndex][FaceIndex] = VertexSkip + Reader.ReadUInt32();
                        }
                    }
                    else
                    {
                        std::cerr << "ERROR: Unknown Face Type in Model!\n";
                    }

                Reader.Seek(MeshBaseOffset + ((LODIndex+1) * 56), BinaryVectorReader::Position::Begin);
            }

            this->m_Models[FMDLIndex].Materials[FSHPIndex] = Materials[MaterialIndex];

            Reader.Seek(FSHPBaseOffset + 96, BinaryVectorReader::Position::Begin);
        }

        this->m_Models[FMDLIndex].Vertices.resize(FVTXCount);
        Reader.Seek(FVTXOffset, BinaryVectorReader::Position::Begin);
        for (int FVTXIndex = 0; FVTXIndex < FVTXCount; FVTXIndex++)
        {
            uint64_t FVTXBaseOffset = Reader.GetPosition();
            Reader.Seek(8, BinaryVectorReader::Position::Current);

            uint64_t FVTXAttArrOffset = Reader.ReadUInt64();
            uint64_t FVTXAttArrDictionary = Reader.ReadUInt64();

            Reader.Seek(FVTXBaseOffset + 48, BinaryVectorReader::Position::Begin);
            uint64_t VertexBufferSizeOffset = Reader.ReadUInt64();
            uint64_t VertexStrideSizeOffset = Reader.ReadUInt64();

            Reader.Seek(8, BinaryVectorReader::Position::Current); //Padding

            uint32_t BufferOffset = Reader.ReadUInt32();
            uint8_t NumVertexAttrib = Reader.ReadUInt8();
            uint8_t NumBuffer = Reader.ReadUInt8();
            uint16_t Idx = Reader.ReadUInt16();
            uint32_t VertexCount = Reader.ReadUInt32();
            uint8_t VertexSkinCount = Reader.ReadUInt8();

            std::vector<BfresFile::VertexBufferSize> VertexBufferSizeArray(NumBuffer);
            std::vector<uint32_t> VertexBufferStrideArray(NumBuffer);
            std::vector<BfresFile::VertexBufferAttribute> VertexBufferAttributes(NumVertexAttrib);

            Reader.Seek(VertexBufferSizeOffset, BinaryVectorReader::Position::Begin);
            for (int i = 0; i < NumBuffer; i++)
            {
                BfresFile::VertexBufferSize VBufferSize;
                VBufferSize.Size = Reader.ReadUInt32();
                VBufferSize.GPUFlags = Reader.ReadUInt32();
                Reader.Seek(8, BinaryVectorReader::Position::Current);
                VertexBufferSizeArray[i] = VBufferSize;
            }

            Reader.Seek(VertexStrideSizeOffset, BinaryVectorReader::Position::Begin);
            for (int i = 0; i < NumBuffer; i++)
            {
                VertexBufferStrideArray[i] = Reader.ReadUInt32();
                Reader.Seek(12, BinaryVectorReader::Position::Current);
            }

            Reader.Seek(FVTXAttArrOffset, BinaryVectorReader::Position::Begin);
            for (int i = 0; i < NumVertexAttrib; i++)
            {
                Reader.Seek(Reader.ReadUInt64(), BinaryVectorReader::Position::Begin);

                uint16_t NameLength = Reader.ReadUInt16();
                for (int j = 0; j < NameLength; j++) {
                    VertexBufferAttributes[i].Name += (char)Reader.ReadInt8();
                }

                Reader.Seek(FVTXAttArrOffset + 16 * i + 8, BinaryVectorReader::Position::Begin);
            
                VertexBufferAttributes[i].Format = Reader.ReadUInt16();
                Reader.Seek(2, BinaryVectorReader::Position::Current);
                VertexBufferAttributes[i].Offset = Reader.ReadUInt16();
                VertexBufferAttributes[i].BufferIndex = Reader.ReadUInt16();

                Reader.Seek(FVTXAttArrOffset + 16 * (i + 1), BinaryVectorReader::Position::Begin);
            }

            std::cout << BufferOffset << std::endl;

            std::vector<BfresFile::VertexBuffer> VertexBuffers(NumBuffer);
            for (int i = 0; i < NumBuffer; i++)
            {
                VertexBuffers[i].Stride = VertexBufferStrideArray[i];
                VertexBuffers[i].Size = VertexBufferSizeArray[i].Size;
                VertexBuffers[i].Data.resize(VertexBuffers[i].Size);

                if (i == 0) VertexBuffers[i].BufferOffset = DataStart + BufferOffset;
                if (i > 0) VertexBuffers[i].BufferOffset = VertexBuffers[i - 1].BufferOffset + VertexBuffers[i - 1].Size;
                if (VertexBuffers[i].BufferOffset % 8 != 0) VertexBuffers[i].BufferOffset = VertexBuffers[i].BufferOffset + (8 - (VertexBuffers[i].BufferOffset % 8));

                Reader.Seek(VertexBuffers[i].BufferOffset, BinaryVectorReader::Position::Begin);

                for (int j = 0; j < VertexBuffers[i].Size; j++)
                {
                    VertexBuffers[i].Data[j] = (unsigned char)Reader.ReadUInt8();
                }
            }

            this->m_Models[FMDLIndex].Vertices[FVTXIndex].resize(VertexCount * 3);

            /* Parsing vertices */
            for (int VertexIndex = 0; VertexIndex < VertexCount; VertexIndex++)
            {
                if (VertexBufferAttributes[0].Format != (uint16_t)BfresFile::VertexBufferFormat::Format_32_32_32_Single)
                {
                    this->m_Models[FMDLIndex].Vertices[FVTXIndex][VertexIndex * 3] = this->ShortToFloat(VertexBuffers[0].Data[VertexIndex * VertexBuffers[0].Stride], VertexBuffers[0].Data[VertexIndex * VertexBuffers[0].Stride + 1]);
                    this->m_Models[FMDLIndex].Vertices[FVTXIndex][VertexIndex * 3 + 1] = this->ShortToFloat(VertexBuffers[0].Data[VertexIndex * VertexBuffers[0].Stride + 2], VertexBuffers[0].Data[VertexIndex * VertexBuffers[0].Stride + 3]);
                    this->m_Models[FMDLIndex].Vertices[FVTXIndex][VertexIndex * 3 + 2] = this->ShortToFloat(VertexBuffers[0].Data[VertexIndex * VertexBuffers[0].Stride + 4], VertexBuffers[0].Data[VertexIndex * VertexBuffers[0].Stride + 5]);
                }
                else
                {
                    this->m_Models[FMDLIndex].Vertices[FVTXIndex][VertexIndex * 3] = this->UInt32ToFloat(VertexBuffers[0].Data[VertexIndex * VertexBuffers[0].Stride], VertexBuffers[0].Data[VertexIndex * VertexBuffers[0].Stride + 1], VertexBuffers[0].Data[VertexIndex * VertexBuffers[0].Stride + 2], VertexBuffers[0].Data[VertexIndex * VertexBuffers[0].Stride + 3]);
                    this->m_Models[FMDLIndex].Vertices[FVTXIndex][VertexIndex * 3 + 1] = this->UInt32ToFloat(VertexBuffers[0].Data[VertexIndex * VertexBuffers[0].Stride + 4], VertexBuffers[0].Data[VertexIndex * VertexBuffers[0].Stride + 5], VertexBuffers[0].Data[VertexIndex * VertexBuffers[0].Stride + 6], VertexBuffers[0].Data[VertexIndex * VertexBuffers[0].Stride + 7]);
                    this->m_Models[FMDLIndex].Vertices[FVTXIndex][VertexIndex * 3 + 2] = this->UInt32ToFloat(VertexBuffers[0].Data[VertexIndex * VertexBuffers[0].Stride + 8], VertexBuffers[0].Data[VertexIndex * VertexBuffers[0].Stride + 9], VertexBuffers[0].Data[VertexIndex * VertexBuffers[0].Stride + 10], VertexBuffers[0].Data[VertexIndex * VertexBuffers[0].Stride + 11]);
                }
                /*
                if(Path.find("HyruleCastleGround") != std::string::npos)
                    std::cout << "Vertice: " << this->m_Models[FMDLIndex].Vertices[FVTXIndex][VertexIndex * 3] << ", " << this->m_Models[FMDLIndex].Vertices[FVTXIndex][VertexIndex * 3 + 1] << ", " << this->m_Models[FMDLIndex].Vertices[FVTXIndex][VertexIndex * 3 + 2] << std::endl;
                */
            }

            uint32_t AttributeSegmentSize = 0;
            VertexBufferAttribute* BiggestOffsetAttribute = nullptr;
            for (VertexBufferAttribute& Attribute : VertexBufferAttributes)
            {
                std::string NameCopy = Attribute.Name;
                NameCopy.pop_back();
                if (NameCopy != "_p" && NameCopy != "_i" && NameCopy != "_w")
                {
                    if (BiggestOffsetAttribute != nullptr)
                    {
                        if (Attribute.Offset > BiggestOffsetAttribute->Offset)
                        {
                            BiggestOffsetAttribute = &Attribute;
                        }
                    }
                    else
                    {
                        BiggestOffsetAttribute = &Attribute;
                    }
                }
            }

            AttributeSegmentSize = BiggestOffsetAttribute->Offset;

            switch (BiggestOffsetAttribute->Format) {
            case (uint16_t)VertexBufferFormat::Format_32_32_Single:
                AttributeSegmentSize += 8;
                break;
            case (uint16_t)VertexBufferFormat::Format_10_10_10_2_SNorm:
            case (uint16_t)VertexBufferFormat::Format_8_8_8_8_SNorm:
            case (uint16_t)VertexBufferFormat::Format_8_8_8_8_UInt:
            case (uint16_t)VertexBufferFormat::Format_8_8_8_8_UNorm:
            case (uint16_t)VertexBufferFormat::Format_16_16_Single:
            case (uint16_t)VertexBufferFormat::Format_16_16_UNorm:
            case (uint16_t)VertexBufferFormat::Format_16_16_SNorm:
                AttributeSegmentSize += 4;
                break;
            case (uint16_t)VertexBufferFormat::Format_8_8_SNorm:
            case (uint16_t)VertexBufferFormat::Format_8_8_UNorm:
                AttributeSegmentSize += 2;
                break;
            default:
                break;
            }

            //Aligning to 4 bytes
            while (AttributeSegmentSize % 4 != 0)
            {
                AttributeSegmentSize++;
            }

            int UVBufferIndex = 0;
            /* Parsing UV Data */
            for (BfresFile::VertexBufferAttribute& Attribute : VertexBufferAttributes)
            {
                if (Attribute.Name[0] == '_' && Attribute.Name[1] == 'u') //UV Buffer Attribute
                {
                    if (!(this->m_Models[FMDLIndex].Materials[FVTXIndex].Textures.size()-1 >= UVBufferIndex)) continue;
                    this->m_Models[FMDLIndex].Materials[FVTXIndex].Textures[UVBufferIndex].TexCoordinates.resize(VertexCount * 2);

                    for (int VertexIndex = 0; VertexIndex < VertexCount; VertexIndex++)
                    {
                        switch (Attribute.Format)
                        {
                        case (uint16_t)VertexBufferFormat::Format_16_16_Single:
                            this->m_Models[FMDLIndex].Materials[FVTXIndex].Textures[UVBufferIndex].TexCoordinates[VertexIndex * 2] = this->ShortToFloat(VertexBuffers[Attribute.BufferIndex].Data[VertexIndex * AttributeSegmentSize + Attribute.Offset], VertexBuffers[Attribute.BufferIndex].Data[VertexIndex * AttributeSegmentSize + Attribute.Offset + 1]);
                            this->m_Models[FMDLIndex].Materials[FVTXIndex].Textures[UVBufferIndex].TexCoordinates[VertexIndex * 2 + 1] = this->ShortToFloat(VertexBuffers[Attribute.BufferIndex].Data[VertexIndex * AttributeSegmentSize + Attribute.Offset + 2], VertexBuffers[Attribute.BufferIndex].Data[VertexIndex * AttributeSegmentSize + Attribute.Offset + 3]);                            
                            break;
                        case (uint16_t)VertexBufferFormat::Format_16_16_UNorm:
                             this->m_Models[FMDLIndex].Materials[FVTXIndex].Textures[UVBufferIndex].TexCoordinates[VertexIndex * 2] = this->CombineToUInt16(VertexBuffers[Attribute.BufferIndex].Data[VertexIndex * AttributeSegmentSize + Attribute.Offset], VertexBuffers[Attribute.BufferIndex].Data[VertexIndex * AttributeSegmentSize + Attribute.Offset + 1]) / 65536.0f;
                             this->m_Models[FMDLIndex].Materials[FVTXIndex].Textures[UVBufferIndex].TexCoordinates[VertexIndex * 2 + 1] = this->CombineToUInt16(VertexBuffers[Attribute.BufferIndex].Data[VertexIndex * AttributeSegmentSize + Attribute.Offset + 2], VertexBuffers[Attribute.BufferIndex].Data[VertexIndex * AttributeSegmentSize + Attribute.Offset + 3]) / 65536.0f;
                            break;
                        case (uint16_t)VertexBufferFormat::Format_16_16_SNorm:
                             this->m_Models[FMDLIndex].Materials[FVTXIndex].Textures[UVBufferIndex].TexCoordinates[VertexIndex * 2] = this->CombineToUInt16(VertexBuffers[Attribute.BufferIndex].Data[VertexIndex * AttributeSegmentSize + Attribute.Offset], VertexBuffers[Attribute.BufferIndex].Data[VertexIndex * AttributeSegmentSize + Attribute.Offset + 1]) / 32768.0f;
                             this->m_Models[FMDLIndex].Materials[FVTXIndex].Textures[UVBufferIndex].TexCoordinates[VertexIndex * 2 + 1] = this->CombineToUInt16(VertexBuffers[Attribute.BufferIndex].Data[VertexIndex * AttributeSegmentSize + Attribute.Offset + 2], VertexBuffers[Attribute.BufferIndex].Data[VertexIndex * AttributeSegmentSize + Attribute.Offset + 3]) / 32768.0f;
                            break;
                        case (uint16_t)VertexBufferFormat::Format_8_8_UNorm:
                             this->m_Models[FMDLIndex].Materials[FVTXIndex].Textures[UVBufferIndex].TexCoordinates[VertexIndex * 2] = (float)VertexBuffers[Attribute.BufferIndex].Data[VertexIndex * AttributeSegmentSize + Attribute.Offset] / 256.0f;
                             this->m_Models[FMDLIndex].Materials[FVTXIndex].Textures[UVBufferIndex].TexCoordinates[VertexIndex * 2 + 1] = (float)VertexBuffers[Attribute.BufferIndex].Data[VertexIndex * AttributeSegmentSize + Attribute.Offset + 1] / 256.0f;
                            break;
                        case (uint16_t)VertexBufferFormat::Format_8_8_SNorm:
                             this->m_Models[FMDLIndex].Materials[FVTXIndex].Textures[UVBufferIndex].TexCoordinates[VertexIndex * 2] = (float)VertexBuffers[Attribute.BufferIndex].Data[VertexIndex * AttributeSegmentSize + Attribute.Offset] / 128.0f;
                             this->m_Models[FMDLIndex].Materials[FVTXIndex].Textures[UVBufferIndex].TexCoordinates[VertexIndex * 2 + 1] = (float)VertexBuffers[Attribute.BufferIndex].Data[VertexIndex * AttributeSegmentSize + Attribute.Offset + 1] / 128.0f;
                            break;
                        case (uint16_t)VertexBufferFormat::Format_32_32_Single:
                             this->m_Models[FMDLIndex].Materials[FVTXIndex].Textures[UVBufferIndex].TexCoordinates[VertexIndex * 2] = this->UInt32ToFloat(VertexBuffers[Attribute.BufferIndex].Data[VertexIndex * AttributeSegmentSize + Attribute.Offset], VertexBuffers[Attribute.BufferIndex].Data[VertexIndex * AttributeSegmentSize + Attribute.Offset + 1], VertexBuffers[Attribute.BufferIndex].Data[VertexIndex * AttributeSegmentSize + Attribute.Offset + 2], VertexBuffers[Attribute.BufferIndex].Data[VertexIndex * AttributeSegmentSize + Attribute.Offset + 3]);
                             this->m_Models[FMDLIndex].Materials[FVTXIndex].Textures[UVBufferIndex].TexCoordinates[VertexIndex * 2 + 1] = this->UInt32ToFloat(VertexBuffers[Attribute.BufferIndex].Data[VertexIndex * AttributeSegmentSize + Attribute.Offset + 4], VertexBuffers[Attribute.BufferIndex].Data[VertexIndex * AttributeSegmentSize + Attribute.Offset + 5], VertexBuffers[Attribute.BufferIndex].Data[VertexIndex * AttributeSegmentSize + Attribute.Offset + 6], VertexBuffers[Attribute.BufferIndex].Data[VertexIndex * AttributeSegmentSize + Attribute.Offset + 7]);
                            break;
                        }
                    }
                    UVBufferIndex++;
                }
            }

            Reader.Seek(FVTXBaseOffset + 88, BinaryVectorReader::Position::Begin);
        }
    }

    this->CreateOpenGLObjects();
}

BfresFile::BfresFile(std::vector<unsigned char> Bytes)
{
    this->BfresFile::BfresFile("", Bytes);
}

BfresFile::BfresFile(std::string Path)
{
    std::ifstream File(Path, std::ios::binary);

    if (!File.eof() && !File.fail())
    {
        File.seekg(0, std::ios_base::end);
        std::streampos FileSize = File.tellg();

        std::vector<unsigned char> Bytes(FileSize);

        File.seekg(0, std::ios_base::beg);
        File.read(reinterpret_cast<char*>(Bytes.data()), FileSize);

        File.close();

        this->BfresFile::BfresFile(Path, Bytes);
    }
    else
    {
        std::cerr << "Could not open file \"" << Path << "\"!\n";
    }
}

void BfresFile::Delete()
{
    if (this->m_Models.size() == 0) return;

    for (BfresFile::LOD& LODModel : this->m_Models[0].LODs)
    {
        for (int SubModelIndex = 0; SubModelIndex < LODModel.Faces.size(); SubModelIndex++)
        {
            LODModel.GL_VBO[SubModelIndex].Delete();
            LODModel.GL_VAO[SubModelIndex].Delete();
            LODModel.GL_EBO[SubModelIndex].Delete();
        }
    }
}