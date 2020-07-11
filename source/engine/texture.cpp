#include "texture.h"
#include "assertion.h"
#include "image_utils.h"
#include "string.h"
#include <GL/glew.h>

Texture Texture::create(const TextureDesc& desc)
{
	asserts(desc.width > 0 && desc.height > 0);

	GLenum format;
	switch (desc.format)
	{
		case TextureFormat::Mono: format = GL_RED; break;
		case TextureFormat::RGB: format = GL_RGB; break;
		case TextureFormat::RGBA: format = GL_RGBA; break;
		default: asserts(false, "unsupported texture format");
	}

	// for one channel texture, may want to swizzle
	// https://stackoverflow.com/questions/37027551/why-do-i-need-to-specify-image-format-as-gl-red-instead-of-gl-alpha-when-loading
	// https://www.khronos.org/opengl/wiki/Texture#Swizzle_mask
	// GLint swizzleMask[] = {GL_ZERO, GL_ZERO, GL_ZERO, GL_RED};
	// glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

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

Id create_texture(const TextureDesc& desc)
{
	asserts(desc.width > 0 && desc.height > 0);

	GLenum format;
	int bpp;
	switch (desc.format)
	{
		case TextureFormat::Mono: format = GL_RED; bpp = 1; break;
		case TextureFormat::RGB: format = GL_RGB; bpp = 3; break;
		case TextureFormat::RGBA: format = GL_RGBA; bpp = 4; break;
		default: asserts(false, "unsupported texture format");
	}

	// for one channel texture, may want to swizzle
	// https://stackoverflow.com/questions/37027551/why-do-i-need-to-specify-image-format-as-gl-red-instead-of-gl-alpha-when-loading
	// https://www.khronos.org/opengl/wiki/Texture#Swizzle_mask
	// GLint swizzleMask[] = {GL_ZERO, GL_ZERO, GL_ZERO, GL_RED};
	// glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

	GLuint id;
	glGenTextures(1, &id);
	GLint align;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &align);

	const bool is_tex_array = (desc.layers > 0);
	GLenum target;
	if (is_tex_array)
	{
		asserts(((desc.width * bpp + align - 1) / align) * align * desc.height * desc.layers == desc.data.size());
		target = GL_TEXTURE_2D_ARRAY;
		glBindTexture(target, id);
		glTexImage3D(target, 0, format, desc.width, desc.height, desc.layers, 0, format, GL_UNSIGNED_BYTE, desc.data.data());
	}
	else
	{
		asserts(((desc.width * bpp + align - 1) / align) * align * desc.height == desc.data.size());
		target = GL_TEXTURE_2D;
		glBindTexture(target, id);
		glTexImage2D(target, 0, format, desc.width, desc.height, 0, format, GL_UNSIGNED_BYTE, desc.data.data());
	}
	
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // TODO: allow customization
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(target, 0);

	return id;
}

Id load_texture(const String& filename)
{
	TextureDesc desc;
	desc.layers = 0;
	auto img = load_image(filename.c_str(), true);
	desc.width = img.width;
	desc.height = img.height;
	desc.format = img.format;
	desc.data = std::move(img.data);
	return create_texture(desc);
}

Id load_texture_array(const std::vector<String>& filenames)
{
	TextureDesc desc;
	desc.layers = 0;
	for (const auto& filename : filenames)
	{
		auto img = load_image(filename.c_str(), true);
		if (desc.layers == 0)
		{
			desc.width = img.width;
			desc.height = img.height;
			desc.format = img.format;
			desc.data = std::move(img.data);
		}
		else
		{
			asserts(desc.width == img.width && desc.height == img.height && desc.format == img.format);
			size_t ptr = desc.data.size();
			desc.data.resize(ptr + img.data.size());
			std::copy(img.data.begin(), img.data.end(), desc.data.begin() + ptr);
		}
		desc.layers++;
	}
	return create_texture(desc);
}

