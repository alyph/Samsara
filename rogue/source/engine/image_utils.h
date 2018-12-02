#pragma once

#include "graphics/texture.h"

struct ImageLoadResult
{
	int width{}, height{};
	TextureFormat format{};
	std::vector<uint8_t> data;
};

extern ImageLoadResult load_image(const char* filename, bool vertical_flip=false);
extern TextureDesc load_texture(const char* filename);


