#pragma once

#include "engine/texture.h"

struct Image
{
	int width{}, height{};
	TextureFormat format{};
	std::vector<uint8_t> data;
};

extern Image load_image(const char* filename, bool vertical_flip=false);
extern TextureDesc load_texture(const char* filename);

extern bool save_image(const char* filename, const Image& image, bool vertical_flip=false);


