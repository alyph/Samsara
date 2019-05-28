#include "image_utils.h"
#include "assertion.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

Image load_image(const char* filename, bool vertical_flip)
{
	Image result;

	stbi_set_flip_vertically_on_load(vertical_flip);

	int x, y, n;
	auto data = stbi_load(filename, &x, &y, &n, 0);

	if (!data)
	{
		printf("Failed to load image: %s", filename);
		return result;
	}

	result.width = x;
	result.height = y;
	
	switch (n)
	{
		case 1: result.format = TextureFormat::Mono; break;
		case 3: result.format = TextureFormat::RGB; break;
		case 4: result.format = TextureFormat::RGBA; break;
		default: asserts(false, "unsupported number of channels");
	}

	size_t size = (y * x * n);
	result.data.resize(size);
	std::copy(data, data + size, result.data.data());
	stbi_image_free(data);
	return result;
}

TextureDesc load_texture(const char* filename)
{
	auto result = load_image(filename, true);

	TextureDesc desc;
	desc.width = result.width;
	desc.height = result.height;
	desc.format = result.format;
	desc.data = std::move(result.data);
	return desc;
}

bool save_image(const char* filename, const Image& image, bool vertical_flip)
{
	stbi_flip_vertically_on_write(vertical_flip);

	int w = image.width;
	int h = image.height;
	int n = 0;

	switch (image.format)
	{
		case TextureFormat::Mono: n = 1; break;
		case TextureFormat::RGB: n = 3; break;
		case TextureFormat::RGBA: n = 4; break;
		default: asserts(false, "unsupported texture format");
	}

	size_t size = (w * h * n);
	asserts(image.data.size() == size);
	return stbi_write_png(filename, w, h, n, image.data.data(), w * n) != 0;
}
