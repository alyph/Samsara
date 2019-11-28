#pragma once

#include "id.h"
#include <vector>

enum class TextureFormat
{
	Mono,
	RGB,
	RGBA,
};

struct TextureDesc
{
	int width{};
	int height{};
	TextureFormat format = TextureFormat::RGBA;
	std::vector<uint8_t> data;
};

// convert texture into id based and manage it through asset manager
class Texture
{
public:

	~Texture();
	static Texture create(const TextureDesc& desc);

	Texture() = default;
	Texture(const Texture& other) = delete;
	Texture& operator=(const Texture& other) = delete;
	Texture(Texture&& other);
	Texture& operator=(Texture&& other);

	Id id() const { return texture_id; }

private:
	Id texture_id{};
};
