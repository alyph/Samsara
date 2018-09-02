#include "image_utils.h"
#include "core/assertion.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

ImageLoadResult load_image(const char* filename, bool vertical_flip)
{
	ImageLoadResult result;

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

