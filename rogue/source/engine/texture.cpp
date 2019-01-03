#include "texture.h"
#include "assertion.h"
#include <GL/glew.h>

Texture Texture::create(const TextureDesc& desc)
{
	asserts(desc.width > 0 && desc.height > 0);

	GLenum format;
	switch (desc.format)
	{
		case TextureFormat::RGB: format = GL_RGB; break;
		case TextureFormat::RGBA: format = GL_RGBA; break;
		default: asserts(false, "unsupported texture format");
	}

	GLuint id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, format, desc.width, desc.height, 0, format, GL_UNSIGNED_BYTE, desc.data.data());
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);

	Texture texture;
	texture.texture_id = id;
	return texture;
}

Texture::~Texture()
{
	GLuint id = static_cast<GLuint>(texture_id);
	glDeleteTextures(1, &id);
}

Texture::Texture(Texture&& other)
{
	*this = std::move(other);
}
Texture& Texture::operator=(Texture&& other)
{
	texture_id = other.texture_id;
	other.texture_id = 0;
	return *this;
}
