#pragma once

#include <utility>
#include "string.h"
#include "array.h"
#include "texture.h"

class Font;

extern void init_font_api();
extern void term_font_api();
extern Font create_font(const String& font_file, unsigned int size_w, unsigned int size_h);
extern Id create_font_texture(int width, int height, int pages, const Array<uint32_t>& codes, const Font& font, int advance=0);

class Font
{
public:
	void* handle{};

	Font() = default;
	Font(const Font& other) = delete;
	inline Font(Font&& other) { *this = std::move(other); }
	inline ~Font();

	Font& operator=(const Font& other) = delete;
	inline Font& operator=(Font&& other);

private:
	void dispose_handle();
};


inline Font::~Font()
{
	if (handle)
	{
		dispose_handle();
	}
}


inline Font& Font::operator=(Font&& other)
{
	if (handle)
	{
		dispose_handle();
	}
	handle = other.handle;
	other.handle = nullptr;
	return *this;
}

