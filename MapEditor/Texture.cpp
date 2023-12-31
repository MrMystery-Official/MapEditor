#include "Texture.h"

Texture::Texture(const char* image, GLenum texType, GLenum slot, GLenum format, GLenum pixelType)
{
	// Assigns the type of the texture ot the texture object
	type = texType;

	// Stores the width, height, and the number of color channels of the image
	int widthImg, heightImg, numColCh;
	// Flips the image so it appears right side up
	stbi_set_flip_vertically_on_load(true);
	// Reads the image from a file and stores it in bytes
	unsigned char* bytes = stbi_load(image, &widthImg, &heightImg, &numColCh, 0);

	// Generates an OpenGL texture object
	glGenTextures(1, &ID);
	// Assigns the texture to a Texture Unit
	glActiveTexture(slot);
	glBindTexture(texType, ID);

	// Configures the type of algorithm that is used to make the image smaller or bigger
	glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Configures the way the texture repeats (if it does at all)
	glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Extra lines in case you choose to use GL_CLAMP_TO_BORDER
	// float flatColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
	// glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, flatColor);

	// Assigns the image to the OpenGL Texture object
	glTexImage2D(texType, 0, GL_RGBA, widthImg, heightImg, 0, format, pixelType, bytes);
	// Generates MipMaps
	glGenerateMipmap(texType);

	// Deletes the image data as it is already in the OpenGL Texture object
	stbi_image_free(bytes);

	// Unbinds the OpenGL Texture object so that it can't accidentally be modified
	glBindTexture(texType, 0);
}

Texture::Texture(std::vector<unsigned char> Pixels, int Width, int Height, GLenum ValueType, GLenum texType, GLenum slot, GLenum format, GLenum pixelType)
{
	// Assigns the type of the texture ot the texture object
	type = texType;

	// Generates an OpenGL texture object
	glGenTextures(1, &ID);
	// Assigns the texture to a Texture Unit
	glActiveTexture(slot);
	glBindTexture(texType, ID);

	// Configures the type of algorithm that is used to make the image smaller or bigger
	glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Configures the way the texture repeats (if it does at all)
	glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Extra lines in case you choose to use GL_CLAMP_TO_BORDER
	// float flatColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
	// glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, flatColor);

	// Assigns the image to the OpenGL Texture object
	glTexImage2D(texType, 0, ValueType, Width, Height, 0, format, pixelType, Pixels.data());
	// Generates MipMaps
	glGenerateMipmap(texType);

	// Unbinds the OpenGL Texture object so that it can't accidentally be modified
	glBindTexture(texType, 0);
}

Texture::Texture(std::vector<TextureToGo>& Textures, GLenum ValueType, GLenum texType, GLenum slot, GLenum format, GLenum pixelType)
{
	int Width = 0;
	int Height = 0;

	for (TextureToGo& TexToGo : Textures)
	{
		if (TexToGo.GetWidth() > Width) Width = TexToGo.GetWidth();
		if (TexToGo.GetHeight() > Height) Height = TexToGo.GetHeight();
	}

	std::vector<unsigned char> Pixels(Width * Height * 4);
	for (TextureToGo& TexToGo : Textures)
	{
		for (int PixelIndex = 0; PixelIndex < TexToGo.GetPixels().size() / 4; PixelIndex++)
		{
			if (TexToGo.GetPixels()[PixelIndex * 4 + 3] == 0) continue;

			if (TexToGo.GetPixels()[PixelIndex * 4 + 3] == 255)
			{
				Pixels[PixelIndex * 4] = TexToGo.GetPixels()[PixelIndex * 4];
				Pixels[PixelIndex * 4 + 1] = TexToGo.GetPixels()[PixelIndex * 4 + 1];
				Pixels[PixelIndex * 4 + 2] = TexToGo.GetPixels()[PixelIndex * 4 + 2];
				Pixels[PixelIndex * 4 + 3] = 255;
			}

			if (Pixels[PixelIndex * 4] == 0) Pixels[PixelIndex * 4] = TexToGo.GetPixels()[PixelIndex * 4];
			if (Pixels[PixelIndex * 4 + 1] == 0) Pixels[PixelIndex * 4 + 1] = TexToGo.GetPixels()[PixelIndex * 4 + 1];
			if (Pixels[PixelIndex * 4 + 2] == 0) Pixels[PixelIndex * 4 + 2] = TexToGo.GetPixels()[PixelIndex * 4 + 2];
			if (Pixels[PixelIndex * 4 + 3] == 0) Pixels[PixelIndex * 4 + 3] = TexToGo.GetPixels()[PixelIndex * 4 + 3];
		}
	}

	this->Texture::Texture(Pixels, Width, Height, ValueType, texType, slot, format, pixelType);
}

Texture::Texture(TextureToGo* TxtToGo, GLenum ValueType, GLenum texType, GLenum slot, GLenum format, GLenum pixelType)
{
	this->Texture::Texture(TxtToGo->GetPixels(), TxtToGo->GetWidth(), TxtToGo->GetHeight(), ValueType, texType, slot, format, pixelType);
}

void Texture::texUnit(Shader& shader, const char* uniform, GLuint unit)
{
	// Gets the location of the uniform
	GLuint texUni = glGetUniformLocation(shader.ID, uniform);
	// Shader needs to be activated before changing the value of a uniform
	shader.Activate();
	// Sets the value of the uniform
	glUniform1i(texUni, unit);
}

void Texture::Bind()
{
	glBindTexture(type, ID);
}

void Texture::Unbind()
{
	glBindTexture(type, 0);
}

void Texture::Delete()
{
	glDeleteTextures(1, &ID);
}