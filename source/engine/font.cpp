#include "font.h"
#include "image_utils.h"
#include "filesystem.h"
#include <ft2build.h>
#include FT_FREETYPE_H

static FT_Library ft_lib;

void init_font_api()
{
	const auto error = FT_Init_FreeType(&ft_lib);
	if (error)
	{
		printf("freetype failed to initialize! error: %d\n", error);
	}
}

void term_font_api()
{
	const auto error = FT_Done_FreeType(ft_lib);
	if (error)
	{
		printf("freetype failed to terminate! error: %d\n", error);
	}
}

Font create_font(const String& font_file, unsigned int size_w, unsigned int size_h)
{
	Font font;
	FT_Face font_face;
	const auto error = FT_New_Face(ft_lib, font_file.c_str(), 0, &font_face);
	if (error)
	{
		printf("failed to load the font %s! error: %d\n", font_file.c_str(), error);
	}
	else
	{
		FT_Set_Pixel_Sizes(font_face, size_w, size_h);
		font.handle = reinterpret_cast<void*>(font_face);
	}
	return font;
}

void Font::dispose_handle()
{
	const auto error = FT_Done_Face(reinterpret_cast<FT_Face>(handle));
	if (error)
	{
		printf("failed to release FT font face! error: %d\n", error);
	}
	handle = nullptr;
}

static int round_fixed26_6_to_pixel(FT_Long fixed_pixels)
{
	return static_cast<int>(std::lround(static_cast<double>(fixed_pixels) / 64.0));
}

Id create_font_texture(int width, int height, int pages, const Array<uint32_t>& codes, const Font& font, int advance)
{
	asserts(width > 0 && height > 0 && pages > 0);
	asserts((width * height * pages) == codes.size());
	FT_Face font_face = reinterpret_cast<FT_Face>(font.handle);

	// TODO: a lot of these +1, -1 thing added to the right and bottom can be removed, if we just consider right and bottom are one pixel outside of the bounding box.
	const int glyph_width = advance > 0 ? advance : (font_face->size->metrics.max_advance >> 6);
	// const int glyph_height = round_fixed26_6_to_pixel(FT_MulFix(font_face->height, font_face->size->metrics.y_scale));
	const int glyph_height = round_fixed26_6_to_pixel(FT_MulFix(font_face->ascender - font_face->descender, font_face->size->metrics.y_scale));
	const int glyph_left = 0;
	const int glyph_right = glyph_left + glyph_width - 1;
	const int glyph_top = round_fixed26_6_to_pixel(FT_MulFix(font_face->ascender, font_face->size->metrics.y_scale));
	const int glyph_bottom = glyph_top - glyph_height + 1;
	asserts(glyph_width > 0 && glyph_height > 0);

	// printf("width: %d, height: %d, left: %d top: %d\n", glyph_width, glyph_height, glyph_left, glyph_top);

	const int num_glyphs = (width * height);//256;
	const int sheet_width = width;//16;	
	const int sheet_height = height;//(num_glyphs / sheet_width);
	asserts(num_glyphs % sheet_width == 0);
	const int pixels_per_row = (sheet_width * glyph_width);
	const int pixel_height = (sheet_height * glyph_height);
	const int pixels_per_page = (pixels_per_row * pixel_height);
	ArrayTemp<uint8_t> bitmap{(size_t)(pixels_per_page * pages)};
	// bitmap.alloc(pixels_per_row * pixel_height);

	memset(bitmap.data(), 0, bitmap.size());

	// uint32_t spec_char_codes[] = { 0x2587, 0x2588, 0x2589, 0x254B };

	for (int page = 0; page < pages; page++)
	{
		const auto page_ptr = (bitmap.data() + page * pixels_per_page);

		for (int i = 0; i < num_glyphs; i++)
		{
			// uint32_t char_code = i;

			// if (i < 4)
			// {
			// 	char_code = spec_char_codes[i];
			// }

			const auto char_code = codes[page * num_glyphs + i];
			const auto char_index = FT_Get_Char_Index(font_face, char_code);
			if (char_index == 0)
			{
				continue;
			}

    		FT_Load_Glyph(font_face, char_index, FT_LOAD_RENDER);
			// FT_Load_Char(font_face, char_code, FT_LOAD_RENDER);

			int clipped_left = std::max(font_face->glyph->bitmap_left, glyph_left);
			int clipped_right = std::min(font_face->glyph->bitmap_left + (int)font_face->glyph->bitmap.width - 1, glyph_right);
			int clipped_top = std::min(font_face->glyph->bitmap_top, glyph_top);
			int clipped_bottom = std::max(font_face->glyph->bitmap_top - (int)font_face->glyph->bitmap.rows + 1, glyph_bottom);
			int clipped_width = (clipped_right - clipped_left + 1);
			int clipped_height = (clipped_top - clipped_bottom + 1);

			if (clipped_width <= 0 || clipped_height <= 0)
			{
				continue;
			}

			int x = ((i % sheet_width) * glyph_width) + clipped_left - glyph_left;
			int y = ((i / sheet_width) * glyph_height) + glyph_top - clipped_top;

			for (int row = 0; row < clipped_height; row++)
			{
				void* dst = page_ptr + pixels_per_row * (pixel_height - 1 - (y + row)) + x;
				void* src = font_face->glyph->bitmap.buffer + 
					font_face->glyph->bitmap.width * (row + font_face->glyph->bitmap_top - clipped_top) +
					(clipped_left - font_face->glyph->bitmap_left);
				memcpy(dst, src, clipped_width);
			}
		}
	}

#if 0
	Image image;
	image.format = TextureFormat::Mono;
	image.width = pixels_per_row;
	image.height = pixel_height;
	size_t size = (image.width * image.height);
	image.data.resize(size);
	memcpy(image.data.data(), bitmap.data(), size);
	filesystem::create_directories("./test");
	save_image("./test/font_texture.png", image, true);
#endif

	TextureDesc tex_desc;
	tex_desc.width = pixels_per_row;
	tex_desc.height = pixel_height;
	tex_desc.layers = pages;
	tex_desc.format = TextureFormat::Mono;
	tex_desc.data.resize(bitmap.size());
	memcpy(tex_desc.data.data(), bitmap.data(), bitmap.size());
	// tex_desc.data = std::move(image.data);

	return create_texture(tex_desc);
}



