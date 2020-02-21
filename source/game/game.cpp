#include "game.h"
#include "engine/window.h"
#include "engine/viewport.h"
#include "engine/tablet.h"
#include "engine/shader.h"
#include "engine/image_utils.h"

struct WaterTileMask
{
	uint8_t codes[256];
	constexpr WaterTileMask(): codes{}
	{
		for (unsigned int i = 0; i < 256; i++)
		{
			uint8_t code = 0;
			uint8_t mask = (uint8_t)i;
			if (mask & 0x01) { code |= (0x02 | 0x08); }
			if (mask & 0x02) { code |= (0x01 | 0x02); }
			if (mask & 0x04) { code |= (0x01 | 0x04); }
			if (mask & 0x08) { code |= (0x02 | 0x04); }
			if (mask & 0x10) { code |= (0x02 | 0x08); }
			if (mask & 0x20) { code |= (0x04 | 0x08); }
			if (mask & 0x40) { code |= (0x01 | 0x04); }
			if (mask & 0x80) { code |= (0x01 | 0x08); }
			if (mask & 0x01) { code &= ~(0x01); }
			if (mask & 0x04) { code &= ~(0x02); }
			if (mask & 0x10) { code &= ~(0x04); }
			if (mask & 0x40) { code &= ~(0x08); }
			codes[i] = code;
		}
	}
};
static const constexpr WaterTileMask water_tile_mask{};

Game::Game()
{
	// TODO: move to asset manager or something
	// create a shader
	ShaderDesc desc;
	desc.vs_path = "../../data/shaders/tablet_vs.gls";
	desc.fs_path = "../../data/shaders/tablet_fs.gls";
	tablet_shader = create_shader(desc);

	desc.vs_path = "../../data/shaders/basic_textured_vs.gls";
	desc.fs_path = "../../data/shaders/basic_textured_fs.gls";
	tablet_screen_shader = create_shader(desc);

	// auto tex_desc = load_texture("../../data/fonts/Anikki_square_32x32.png");
	// atlas_texture = Texture::create(tex_desc);
	atlas_texture = load_texture_array(
	{
		"../../data/fonts/cp437_24x24.png",
		"../../data/fonts/anikki_square_24x24.png",
		"../../data/fonts/urr_a_24x24.png",
	});

	world_meta.ex_tile_glyphes = 
	{ 
		0x0200, 0x02ba, 0x02cd, 0x02c8,
		0x02ba, 0x02ba, 0x02c9, 0x02cc,
		0x02cd, 0x02bc, 0x02cd, 0x02ca,
		0x02bb, 0x02b9, 0x02cb, 0x02ce,
	};

	// tile_types.alloc_stored(0, 64);
	world_meta.tile_types = 
	{
		{"empty", 0, 0_rgb, 0_rgb},
		{"forest", 0x0105, 0x30ff50_rgb, 0x30ff50_rgb},
		{"hill", 0x0218, 0x606070_rgb, 0x606070_rgb},
		{"coast", 0x027e, 0x80c0f0_rgb, 0x80c0f0_rgb, TileTypeFlags::water},
	};

	editor_state.selected_tile_type = 1;
	editor_state.map_vp.x = editor_state.map_vp.y = (map_chunk_size / 2);
	map.chunks.alloc_stored(0, 1024);
	map.tiles.alloc_stored(0, 512 * 512);
	map.set_tile({0, 0}, {1});
	map.set_tile({16, 16}, {1});
	map.set_tile({10, 10}, {1});
	map.set_tile({10, 12}, {2});
	map.set_tile({11, 12}, {2});
}


void Game::update()
{
}

static inline uint8_t calc_neighbor_water_mask(const Map& map, const std::vector<TileType>& tile_types, const IVec2& tile_coords)
{
	const IVec2 offsets[] = { {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1} };
	uint8_t mask = 0;
	for (int i = 0; i < 8; i++)
	{
		const Id id = map.tile_id(tile_coords + offsets[i]);
		const uint16_t type = (id ? map.tiles[id_to_index(id)].type : 0);
		if (!(tile_types[type].flags & TileTypeFlags::water))
		{
			mask |= (1 << i);
		}
	}
	return mask;
}

static Id map_view(const Context ctx, Map& map, const WorldMeta& meta, const EditorState& state, const Scalar& width, const Scalar& height)
{
	const Id elem_id = make_element(ctx, null_id);
	_attr(attrs::width, width);
	_attr(attrs::height, height);

	TabletRenderBuffer& render_buffer = access_tablet_render_buffer(ctx);
	const TabletLayout& layout = get_elem_attr_or_assert(*ctx.frame, elem_id, attrs::tablet_layout);
	int map_x = state.map_vp.x - layout.width / 2;
	int map_y = state.map_vp.y - layout.height / 2;

	for (int y = 0; y < layout.height; y++)
	{
		for (int x = 0; x < layout.width; x++)
		{
			IVec2 tile_coords{ map_x + x, map_y + y };
			Id tile_id = map.tile_id(tile_coords);
			if (tile_id)
			{
				const Tile& tile = map.tiles[id_to_index(tile_id)];
				if (tile.type)
				{
					const TileType& tile_type = meta.tile_types[tile.type];					
					uint16_t glyph_code = tile_type.glyph;
					if (tile_type.flags & TileTypeFlags::water)
					{
						const auto mask = calc_neighbor_water_mask(map, meta.tile_types, tile_coords);
						if (mask)
						{
							glyph_code = meta.ex_tile_glyphes[tile_type.ex_glyph_start + water_tile_mask.codes[mask]];
						}
					}

					GlyphData glyph;
					glyph.code = glyph_code;
					glyph.color2 = to_color32(tile_type.color_a);
					glyph.coords = { layout.left + x, layout.top + y };
					render_buffer.push_glyph(elem_id, glyph);
				}
			}
		}
	}

	if (_hover)
	{
		const IVec2& cursor = ctx.frame->curr_input.mouse_hit.iuv;
		if (cursor.x >= layout.left && cursor.x < (layout.left + layout.width) &&
			cursor.y >= layout.top && cursor.y < (layout.top + layout.height))
		{
			const uint16_t tile_type_idx = state.selected_tile_type;
			const TileType& tile_type = meta.tile_types[tile_type_idx];
			GlyphData glyph;
			glyph.code = tile_type.glyph;
			glyph.color2 = to_color32(tile_type.color_a);
			glyph.color1 = to_color32(0x303030_rgb);
			glyph.coords = { cursor.x, cursor.y };
			render_buffer.push_glyph(elem_id, glyph);

			if (_down)
			{
				map.set_tile({map_x + cursor.x - layout.left, map_y + cursor.y - layout.top}, {tile_type_idx});
			}
		}
	}

	return elem_id;
}

static void tile_palette(const Context ctx, const std::vector<TileType>& tile_types, EditorState& state)
{
	int size = 3;
	int cols = 2;

	for (int i = 0; i < tile_types.size(); i++)
	{
		int row = (i / cols);
		int col = (i % cols);

		const Id button_id = elem::node(_ctx_id(i));
		_attr(attrs::placement, ElementPlacement::loose);
		_attr(attrs::left, col * size);
		_attr(attrs::top, row * size);
		_attr(attrs::width, size);
		_attr(attrs::height, size);

		const bool selected = (state.selected_tile_type == i);

		// TODO: we should make a helper to get both render buffer and layout
		// otherwise user can cache a render_buffer outside of the loop and reuse it
		// while the layout has not been properly finalized
		TabletRenderBuffer& render_buffer = access_tablet_render_buffer(ctx);
		const TabletLayout& layout = get_elem_attr_or_assert(*ctx.frame, button_id, attrs::tablet_layout);

		const TileType& type_data = tile_types[i];
		GlyphData glyph;
		int x{layout.left}, y{layout.top};

		auto draw = [&](uint16_t code, const IVec2& coords)
		{
			glyph.code = code;
			glyph.coords = coords;
			render_buffer.push_glyph(button_id, glyph);
		};

		// TODO: maybe allow pushing a static array of glyphs
		// center glyph
		glyph.color2 = to_color32(type_data.color_a);
		draw(type_data.glyph, {x + 1, y + 1});

		uint16_t normal_border[] = { 0x00da, 0x00c4, 0x00bf, 0x00b3, 0x00d9, 0x00c0 };
		uint16_t selected_border[] = { 0x00c9, 0x00cd, 0x00bb, 0x00ba, 0x00bc, 0x00c8 };
		uint16_t* border_codes = selected? selected_border : normal_border;

		// border
		glyph.color2 = 0xd0d0d0_rgb32;
		draw(border_codes[0], {x, y});
		draw(border_codes[1], {x + 1, y});
		draw(border_codes[2], {x + 2, y});
		draw(border_codes[3], {x + 2, y + 1});
		draw(border_codes[4], {x + 2, y + 2});
		draw(border_codes[1], {x + 1, y + 2});
		draw(border_codes[5], {x, y + 2});
		draw(border_codes[3], {x, y + 1});

		// happen next frame
		if (_clicked)
		{
			state.selected_tile_type = i;
		}
	}
}

void Game::present(const Context& ctx)
{
	using namespace elem;

	auto window = engine().window;

	const float aspect = window->aspect();

	int tablet_width = 100;
	int tablet_height = 80;
	const Vec2 tablet_size = calc_tablet_size(tablet_width, tablet_height, atlas_texture);

	const float vp_width = (float)tablet_width;
	const float vp_height = vp_width / aspect;

	int map_height = std::min(tablet_height, (int)std::ceil(vp_height / (tablet_size.y / tablet_height)));

	// TODO: use othographic projection
	Viewpoint vp;
	vp.projection = make_orthographic(vp_width / 2, aspect, 0.f, 100.f);
	vp.pose.pos.y = ((float)tablet_size.y - vp_height) / 2;

	viewport(_ctx);
	_attr(attrs::width, window->width());
	_attr(attrs::height, window->height());
	_attr(attrs::viewpoint, vp);
	_attr(attrs::background_color, Color{});

	_children
	{
		tablet(_ctx);

		
		_attr(attrs::transform, Mat44::identity());
		_attr(attrs::width, tablet_width);
		_attr(attrs::height, tablet_height);
		_attr(attrs::texture, atlas_texture);
		_attr(attrs::shader, tablet_shader);
		_attr(attrs::quad_shader, tablet_screen_shader);

		_children
		{
			map_view(_ctx, map, world_meta, editor_state, tablet_width, map_height);
			tile_palette(_ctx, world_meta.tile_types, editor_state);
		}
	}
}

bool Game::ended()
{
	return engine().window->should_close();
}
