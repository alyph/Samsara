#pragma once

#include "id.h"
#include <vector>

class String;

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
	int layers{}; // == 0 means single texture, >= 1 means texture array
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

extern Id create_texture(const TextureDesc& desc);
extern Id load_texture(const String& filename);
extern Id load_texture_array(const std::vector<String>& filenames);

