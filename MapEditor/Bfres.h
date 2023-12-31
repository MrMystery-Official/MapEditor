#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <stb/stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "Texture.h"
#include "BinaryVectorReader.h"
#include "TextureToGo.h"
#include "Vector3F.h"
#include "Util.h"

class BfresFile
{
public:
	static BfresFile CreateDefaultModel(std::string DefaultName, uint8_t Red, uint8_t Green, uint8_t Blue, uint8_t Alpha);

	struct BfresTexture
	{
		TextureToGo* Texture;
		std::vector<float> TexCoordinates;
	};

	struct Material
	{
		std::string Name;
		bool IsTransparent = false;
		std::vector<BfresFile::BfresTexture> Textures;
	};

	struct LOD
	{
		std::vector<std::vector<unsigned int>> Faces;
		std::vector<VAO> GL_VAO;
		std::vector<VBO> GL_VBO;
		std::vector<EBO> GL_EBO;
		std::vector<Texture*> GL_Textures;
	};

	struct Model
	{
		std::vector<LOD> LODs;
		std::vector<std::vector<float>> Vertices;
		std::vector<Material> Materials;
		float BoundingBoxSphereRadius;
	};

	std::vector<BfresFile::Model>& GetModels();
	bool& IsDefaultModel();
	void Delete();

	BfresFile(std::vector<unsigned char> Bytes);
	BfresFile(std::string Path, std::vector<unsigned char> Bytes);
	BfresFile(std::string Path);
	BfresFile() {}
private:

	enum class VertexBufferFormat : uint16_t
	{
		Format_16_16_16_16_Single = 5381,
		Format_10_10_10_2_SNorm = 3586,
		Format_8_8_8_8_UInt = 2819,
		Format_8_8_8_8_SNorm = 2818,
		Format_8_8_8_8_UNorm = 2817,
		Format_8_8_UNorm = 2305,
		Format_8_8_SNorm = 2306,
		Format_16_16_Single = 4613,
		Format_16_16_UNorm = 4609,
		Format_16_16_SNorm = 4610,
		Format_32_32_Single = 5893,
		Format_32_32_32_Single = 6149
	};

	struct VertexBufferSize
	{
		uint32_t Size;
		uint32_t GPUFlags;
	};

	struct VertexBufferAttribute
	{
		std::string Name;
		uint16_t Format;
		uint16_t Offset;
		uint16_t BufferIndex;
	};

	struct VertexBuffer {
		uint32_t BufferOffset;
		uint32_t Size;
		uint32_t Stride;
		std::vector<unsigned char> Data;
	};

	std::vector<BfresFile::Model> m_Models;
	bool m_DefaultModel = false;
	std::string m_Path = "";

	void GenerateBoundingBox();
	float ShortToFloat(uint8_t Byte1, uint8_t Byte2);
	float UInt32ToFloat(unsigned char Byte1, unsigned char Byte2, unsigned char Byte3, unsigned char Byte4);
	uint16_t CombineToUInt16(uint8_t Byte1, uint8_t Byte2);
	std::string ReadString(BinaryVectorReader& Reader, uint64_t Offset);
	void CreateOpenGLObjects();
};

namespace GLTextureLibrary
{
	extern std::map<TextureToGo*, Texture> Textures;

	bool IsTextureLoaded(TextureToGo* TexToGo);
	Texture* GetTexture(TextureToGo* TexToGo);
	std::map<TextureToGo*, Texture>& GetTextures();
};