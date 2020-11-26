
#include "tablet_ui.h"
#include "engine/app.h"
#include "engine/viewport.h"
#include "engine/tablet.h"
#include "engine/shader.h"
#include "engine/math_utils.h"
#include "engine/image_utils.h"
#include "engine/font.h"
// #include <ft2build.h>
// #include FT_FREETYPE_H
//#include FT_GLYPH_H

int main()
{
	return run_app<TabletUIApp>();
}

// static int round_fixed26_6_to_pixel(FT_Long fixed_pixels)
// {
// 	return static_cast<int>(std::lround(static_cast<double>(fixed_pixels) / 64.0));
// }

TabletUIApp::TabletUIApp()
{
	// create a shader
	ShaderDesc desc;
	desc.vs_path = "../../data/shaders/tablet_vs.gls";
	desc.fs_path = "../../data/shaders/tablet_fs.gls";
	tablet_shader = create_shader(desc);

	desc.vs_path = "../../data/shaders/basic_textured_vs.gls";
	desc.fs_path = "../../data/shaders/basic_textured_fs.gls";
	tablet_screen_shader = create_shader(desc);

	// TODO: the atlas texture should probably use nearest filter 
	// since we are drawing into pixel perfect render buffer
	// atlas_texture = load_texture_array({"../../data/fonts/cp437_20x20.png"});


#if 0
	FT_Library  ft_lib;
	FT_Error error = FT_Init_FreeType(&ft_lib);
	if (!error)
	{
		FT_Face font_face;
		error = FT_New_Face(ft_lib, "../../data/fonts/DejaVuSansMono.ttf", 0, &font_face);
		if (!error)
		{
			FT_Set_Pixel_Sizes(font_face, 0, 36);
			
			
			printf("font loaded.\n");
			printf("- num glyphs: %d\n", font_face->num_glyphs);

#if 0
			printf("- bbox: x: %d ~ %d, y: %d ~ %d\n", font_face->bbox.xMin, font_face->bbox.xMax, font_face->bbox.yMin, font_face->bbox.yMax);
			printf("- height: %d\n", font_face->height);
			printf("- max advances: %d, %d\n", font_face->max_advance_width, font_face->max_advance_height);

			printf("- advance (width): %d\n", font_face->size->metrics.max_advance >> 6);
			printf("- line spacing (height): %d\n", font_face->size->metrics.height >> 6);
			printf("- ascender: %d\n", font_face->size->metrics.ascender >> 6);
			printf("- descender: %d\n", font_face->size->metrics.descender >> 6);

			auto scaled_ascend = FT_MulFix(font_face->ascender, font_face->size->metrics.y_scale);
			auto scaled_descend = FT_MulFix(font_face->descender, font_face->size->metrics.y_scale);
			auto scaled_height = FT_MulFix(font_face->height, font_face->size->metrics.y_scale);
			auto scaled_block_ascend = FT_MulFix(1921, font_face->size->metrics.y_scale);

			printf("- scaled ascender: %d . %d\n", scaled_ascend / 64, scaled_ascend % 64);
			printf("- scaled descender: %d . %d\n", scaled_descend / 64, scaled_descend % 64);
			printf("- scaled height: %d . %d\n", scaled_height / 64, scaled_height % 64);
			printf("- scaled block ascender: %d . %d\n", scaled_block_ascend >> 6, scaled_block_ascend & 63);

			uint32_t char_codes[] = { 33, 0x2587, 0x2588, 0x2589, 0x254B };
			for (size_t i = 0; i < 5; i++)
			{
				uint32_t char_code = char_codes[i];
				FT_Load_Char(font_face, char_code, FT_LOAD_RENDER);

				// FT_BBox bbox;
				// FT_Glyph_Get_CBox(font_face->glyph, FT_GLYPH_BBOX_PIXELS, bbox);
				printf("-- loaded %d\n", char_code);
				printf("- width: %d, height: %d\n", font_face->glyph->metrics.width >> 6, font_face->glyph->metrics.height >> 6);
				printf("- advance horiz: %d, vert: %d\n", font_face->glyph->metrics.horiAdvance >> 6, font_face->glyph->metrics.vertAdvance >> 6);
				printf("- horiz bearing x: %d, y: %d\n", font_face->glyph->metrics.horiBearingX >> 6, font_face->glyph->metrics.horiBearingY >> 6);
				printf("- vert bearing x: %d, y: %d\n", font_face->glyph->metrics.vertBearingX >> 6, font_face->glyph->metrics.vertBearingY >> 6);
				printf("- advance: %d, %d\n", font_face->glyph->advance.x >> 6, font_face->glyph->advance.y >> 6);
				printf("- bitmap: width: %d, rows: %d\n", font_face->glyph->bitmap.width, font_face->glyph->bitmap.rows);
				printf("- bitmap: left: %d, top: %d\n", font_face->glyph->bitmap_left, font_face->glyph->bitmap_top);
			// ▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▉
			// ▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▉▉▉

				Image image;
				image.format = TextureFormat::Mono;
				image.width = font_face->glyph->bitmap.width;
				image.height = font_face->glyph->bitmap.rows;
				size_t size = (image.width * image.height);
				image.data.resize(size);
				std::copy(font_face->glyph->bitmap.buffer, font_face->glyph->bitmap.buffer + size, image.data.data());

				save_image((std::string("d:/test/font_glyph_") + std::to_string(i) + ".png").c_str(), image);
			}

#endif

			const int glyph_width = (font_face->size->metrics.max_advance >> 6);
			const int glyph_height = round_fixed26_6_to_pixel(FT_MulFix(font_face->height, font_face->size->metrics.y_scale));
			const int glyph_left = 0;
			const int glyph_right = glyph_left + glyph_width - 1;
			const int glyph_top = round_fixed26_6_to_pixel(FT_MulFix(font_face->ascender, font_face->size->metrics.y_scale));
			const int glyph_bottom = glyph_top - glyph_height + 1;

			printf("width: %d, height: %d, left: %d top: %d\n", glyph_width, glyph_height, glyph_left, glyph_top);


			const int num_glyphs = 256;
			const int sheet_width = 16;	
			const int sheet_height = (num_glyphs / sheet_width);
			asserts(num_glyphs % sheet_width == 0);
			const int pixels_per_row = (sheet_width * glyph_width);	
			const int pixel_height = (sheet_height * glyph_height);
			ArrayTemp<uint8_t> bitmap;
			bitmap.alloc(pixels_per_row * pixel_height);

			memset(bitmap.data(), 0, bitmap.size());

			uint32_t spec_char_codes[] = { 0x2587, 0x2588, 0x2589, 0x254B };

			for (int i = 0; i < num_glyphs; i++)
			{
				uint32_t char_code = i;

				if (i < 4)
				{
					char_code = spec_char_codes[i];
				}

				FT_Load_Char(font_face, char_code, FT_LOAD_RENDER);

				int clipped_left = std::clamp(font_face->glyph->bitmap_left, glyph_left, glyph_right);
				int clipped_right = std::clamp(font_face->glyph->bitmap_left + (int)font_face->glyph->bitmap.width - 1, glyph_left, glyph_right);
				int clipped_top = std::clamp(font_face->glyph->bitmap_top, glyph_bottom, glyph_top);
				int clipped_bottom = std::clamp(font_face->glyph->bitmap_top - (int)font_face->glyph->bitmap.rows + 1, glyph_bottom, glyph_top);
				int clipped_width = (clipped_right - clipped_left + 1);
				int clipped_height = (clipped_top - clipped_bottom + 1);

				int x = ((i % sheet_width) * glyph_width) + clipped_left - glyph_left;
				int y = ((i / sheet_width) * glyph_height) + glyph_top - clipped_top;

				for (int row = 0; row < clipped_height; row++)
				{
					void* dst = bitmap.data() + pixels_per_row * (pixel_height - 1 - (y + row)) + x;
					void* src = font_face->glyph->bitmap.buffer + 
						font_face->glyph->bitmap.width * (row + font_face->glyph->bitmap_top - clipped_top) +
						(clipped_left - font_face->glyph->bitmap_left);
					memcpy(dst, src, clipped_width);
				}
			}

			Image image;
			image.format = TextureFormat::Mono;
			image.width = pixels_per_row;
			image.height = pixel_height;
			size_t size = (image.width * image.height);
			image.data.resize(size);
			memcpy(image.data.data(), bitmap.data(), size);

			save_image("./test/font_texture.png", image, true);

			TextureDesc tex_desc;
			tex_desc.width = image.width;
			tex_desc.height = image.height;
			tex_desc.format = image.format;
			tex_desc.data = std::move(image.data);
			tex_desc.layers = 1;

			atlas_texture = create_texture(tex_desc);
		}
		else
		{
			printf("failed to load the font!\n");
		}
	}
	else
	{
		printf("freetype failed to init!\n");
	}
#endif

	const auto font = create_font("../../data/fonts/DejaVuSansMono.ttf", 0, 36);

	const int w{16}, h{16}, pages{1};
	const int num = (w * h * pages);
	ArrayTemp<uint32_t> codes{(size_t)num};

	for (int i = 0; i < num; i++)
	{
		codes[i] = i;
	}

	atlas_texture = create_font_texture(w, h, pages, codes, font);
}

void TabletUIApp::update()
{
}

void TabletUIApp::present(const Context& ctx)
{
	using namespace elem;

	auto window = engine().window;

	const float aspect = window->aspect();

	const int tablet_cols = 160;
	const int tablet_rows = 80;
	const float tablet_width = (float)tablet_cols;
	const float tablet_height = calc_tablet_height(tablet_cols, tablet_rows, tablet_width, atlas_texture);

	const float vp_width = (float)tablet_width;
	const float vp_height = vp_width / aspect;

	// TODO: use othographic projection
	Viewpoint vp;
	vp.projection = make_orthographic(vp_width / 2, aspect, 0.f, 100.f);
	vp.pose.pos.y = (tablet_height - vp_height) / 2;

	viewport(_ctx);
	_attr(attrs::width, static_cast<double>(window->width()));
	_attr(attrs::height, static_cast<double>(window->height()));
	_attr(attrs::viewpoint, vp);
	_attr(attrs::background_color, Color{});

	_children
	{
		tablet(_ctx);

		
		_attr(attrs::transform, Mat44::identity());
		_attr(attrs::width, tablet_width);
		_attr(attrs::height, tablet_height);
		_attr(attrs::tablet_columns, tablet_cols);
		_attr(attrs::tablet_rows, tablet_rows);
		_attr(attrs::texture, atlas_texture);
		_attr(attrs::shader, tablet_shader);
		_attr(attrs::quad_shader, tablet_screen_shader);

		_children
		{
			node(_ctx);
			_attr(attrs::height, 2);

			node(_ctx);
			String str = "Dream Park is a futuristic amusement park using holograms and other advanced technologies to entertain customers, including live-action role-players. Dream Park, The Barsoom Project and The California Voodoo Game follow security chief Alex Griffin as he attempts to solve various mysteries set in the park. The other stories in this series have only a peripheral connection. Saturn's Race is a prequel to Achilles' Choice; both involve young adults technologically \"upgrading\" their bodies in an effort to join the world's ruling elite.";
			_attr(attrs::text, str);
			_attr(attrs::foreground_color, (Color{0.8f, 0.1f, 0.5f, 1.f}));

			node(_ctx);
			str = "---------------------------------------------------------------------";
			_attr(attrs::text, str);
			_attr(attrs::foreground_color, (Color{0.8f, 0.8f, 0.8f, 1.f}));


			node(_ctx);
			str = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890~!@#$%^&*()_+-*/={}'\"?<>|\\[]:,.";
			_attr(attrs::text, str);
			_attr(attrs::foreground_color, (Color{0.2f, 0.9f, 0.7f, 1.f}));
			_attr(attrs::width, 50);


			node(_ctx);
			str = format_str("[ click me! (%d) ]", click_count);
			_attr(attrs::text, str);
			
			if (_down)
			{
				_attr(attrs::foreground_color, (Color{1.f, 1.f, 1.f, 1.f}));
				_attr(attrs::background_color, (Color{1.f, 0.5f, 0.5f, 1.f}));
			}
			else if (_hover)
			{
				_attr(attrs::foreground_color, (Color{1.f, 1.f, 1.f, 1.f}));
				_attr(attrs::background_color, (Color{0.8f, 0.1f, 0.1f, 1.f}));
			}
			else
			{
				_attr(attrs::foreground_color, (Color{0.8f, 0.1f, 0.1f, 1.f}));
				_attr(attrs::background_color, (Color{1.f, 1.f, 1.f, 1.f}));
			}

			if (_clicked)
			{
				click_count++;
			}
			

		}
	}

	// auto vp = []() { static Id my_root_id = presenter.new_id(); return viewport(presenter, my_root_id); }();

	// _elem(viewport); _group
	// {
	// 	root_tab = _elem(tablet);
		
	// 	CLICK {  }
	// 	HOVER {  }

	// 	_group
	// 	{
	// 		_elem(pane); _group
	// 		{
	// 			_elem(pane); _group
	// 			{
	// 				_elem(text);
	// 				_elem(border);
	// 			}
	// 			_elem(button);
	// 		}	
	// 	}
	// }
}

bool TabletUIApp::ended()
{
	return engine().window->should_close();
}


	  