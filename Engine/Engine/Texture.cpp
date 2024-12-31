#include "Texture.h"
#include <string.h>
#include <stdlib.h>

Texture::Texture(GLuint id, int width, int height, const char* path) : Resource(ResourceType::TEXTURE), textureId(id), textureWidth(width), textureHeight(height), texturePath(nullptr)
{
	if (path != nullptr)
	{
		size_t len = strlen(path) + 1;
		this->texturePath = (char*)malloc(len);

		if (this->texturePath != nullptr)
			strcpy_s(this->texturePath, len, path);
	}
}

Texture::~Texture()
{
	if (this->texturePath != nullptr)
		free(this->texturePath);
}