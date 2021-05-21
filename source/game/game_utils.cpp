#include "game_utils.h"
#include "engine/font.h"

Id create_font_glyph_page(uint32_t begin_code, const Font& font)
{
	const int w{16}, h{16}, pages{1};
	const int num = (w * h * pages);
	ArrayTemp<uint32_t> codes{(size_t)num};
	for (int i = 0; i < num; i++)
	{
		codes[i] = begin_code + i;
	}
	return create_font_texture(w, h, pages, codes, font);
}



