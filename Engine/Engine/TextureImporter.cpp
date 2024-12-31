#include "TextureImporter.h"
#include "Logger.h"

#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

#include <vector>
#include <fstream>

TextureImporter::TextureImporter()
{
	ilInit();
	iluInit();
	ilutInit();
}

TextureImporter::~TextureImporter()
{
}

void TextureImporter::SaveTextureFile(Resource* resource)
{
	ILuint imageID;
	ilGenImages(1, &imageID);
	ilBindImage(imageID);

	ilLoadImage(resource->GetAssetFileDir().c_str());

	ilSetInteger(IL_DXTC_FORMAT, IL_DXT5);
	ILuint size = ilSaveL(IL_DDS, nullptr, 0);

	std::vector<ILubyte> data(size);
	if (ilSaveL(IL_DDS, data.data(), size) > 0)
	{
		std::string filePath = resource->GetLibraryFileDir();
		std::ofstream file(filePath, std::ios::binary);
		if (file.is_open())
		{
			file.write(reinterpret_cast<char*>(data.data()), size);
			file.close();
		}
	}

	ilDeleteImages(1, &imageID);
}

Texture* TextureImporter::LoadTextureImage(Resource* resource)
{
	ILuint image;
	ilGenImages(1, &image);
	ilBindImage(image);

	if (!ilLoadImage(resource->GetLibraryFileDir().c_str()))
	{
		LOG(LogType::LOG_WARNING, "Image not loaded");
		return nullptr;
	}

	Texture* newTexture = dynamic_cast<Texture*>(resource);
	
	newTexture->textureId = ilutGLBindTexImage();
	newTexture->textureWidth = ilGetInteger(IL_IMAGE_WIDTH);
	newTexture->textureHeight = ilGetInteger(IL_IMAGE_HEIGHT);

	ilDeleteImages(1, &image);

	return newTexture;
}

GLuint TextureImporter::LoadIconImage(const std::string& filePath)
{
	ilClearColour(255, 255, 255, 255);

	ILuint imageID;
	ilGenImages(1, &imageID);
	ilBindImage(imageID);

	if (ilLoadImage(filePath.c_str()) == IL_FALSE)
	{
		ilDeleteImages(1, &imageID);
		return 0;
	}

	ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ilGetInteger(IL_IMAGE_WIDTH),
		ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGBA, GL_UNSIGNED_BYTE, ilGetData());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	ilDeleteImages(1, &imageID);

	return textureID;
}