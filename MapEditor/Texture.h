#pragma once

#include <glad/glad.h>
#include <vector>
#include <stb/stb_image.h>

#include "TextureToGo.h"
#include "Shader.h"

class Texture
{
public:
	GLuint ID;
	GLenum type;
	Texture(const char* image, GLenum texType, GLenum slot, GLenum format, GLenum pixelType);
	Texture(std::vector<unsigned char> Pixels, int Width, int Height, GLenum ValueType, GLenum texType, GLenum slot, GLenum format, GLenum pixelType);
	Texture(std::vector<TextureToGo>& Textures, GLenum ValueType, GLenum texType, GLenum slot, GLenum format, GLenum pixelType);
	Texture(TextureToGo* TxtToGo, GLenum ValueType, GLenum texType, GLenum slot, GLenum format, GLenum pixelType);
	Texture() {};

	// Assigns a texture unit to a texture
	void texUnit(Shader& shader, const char* uniform, GLuint unit);
	// Binds a texture
	void Bind();
	// Unbinds a texture
	void Unbind();
	// Deletes a texture
	void Delete();
};