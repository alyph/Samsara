#include "game.h"
#include "engine/window.h"
#include "engine/viewport.h"
#include "engine/tablet.h"
#include "engine/shader.h"
#include "engine/image_utils.h"
#include "easy/profiler.h"

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
		"../../data/fonts/path_24x24.png",
	});

	globals.terrain_types.alloc_perm( 
	{
		{"empty", 0, 0_rgb32, 0_rgb32},
		{"forest", 0x0105, 0x30ff50_rgb32, 0x30ff50_rgb32},
		{"hill", 0x0218, 0x606070_rgb32, 0x606070_rgb32},
		{"coast", 0x0300, 0x103060_rgb32, 0x80c0f0_rgb32, TileTypeFlags::water},
	});

	editor_state.selected_tile_type = 1;
	editor_state.map_vp.x = editor_state.map_vp.y = (map_chunk_size / 2);
	// TODO: maybe should be in Map's constructor
	world.map.chunks.alloc(0, 1024, stage_allocator());
	world.map.tiles.alloc(0, 512 * 512, stage_allocator());
	world.map.ground_glyphs.alloc(0, 512 * 512, stage_allocator());
}


void Game::update()
{
}



static void paint_square(Map& map, uint16_t tile_type, const IVec2& center, int radius, const Globals& globals)
{
	// TODO: move this function to map and let it do less glyph update
	int r = (radius - 1);
	const Tile tile{tile_type};
	for (int dx = -r; dx <= r; dx++)
	{
		for (int dy = -r; dy <= r; dy++)
		{
			map.set_tile(center + IVec2{dx, dy}, tile, globals);
		}
	}	
}

static void paint_line(Map& map, uint16_t tile_type, const IVec2& start, const IVec2& end, int radius, const Globals& globals)
{
	// https://en.wikipedia.org/wiki/Bresenham's_line_algorithm#All_cases
	// http://members.chello.at/~easyfilter/Bresenham.pdf

	const auto dx = std::abs(end.x - start.x);
	const auto dy = -std::abs(end.y - start.y);
	const auto sx = (start.x < end.x ? 1 : -1);
	const auto sy = (start.y < end.y ? 1 : -1);
	auto err = (dx + dy);
	auto x = start.x;
	auto y = start.y;
	paint_square(map, tile_type, {x, y}, radius, globals);
	while (!((x == end.x && y == end.y)))
	{
		const auto e2 = err * 2;
		if (e2 >= dy) { err += dy; x += sx; }
		if (e2 <= dx) { err += dx; y += sy; }
		// TODO: if this gets too expensive, can just draw the deltas
		paint_square(map, tile_type, {x, y}, radius, globals);
	}
}

static Id map_view(const Context ctx, Map& map, const Globals& globals, EditorState& state, const Scalar& width, const Scalar& height)
{
	EASY_FUNCTION();

	const Id elem_id = make_element(ctx, null_id);
	_attr(attrs::width, width);
	_attr(attrs::height, height);

	TabletLayout layout;
	TabletRenderBuffer& render_buffer = access_tablet_render_buffer_and_layout(ctx, layout);
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
				const auto& ground_glyph = map.ground_glyphs[id_to_index(tile_id)];
				if (ground_glyph.code > 0)
				{
					GlyphData glyph;
					glyph.code = ground_glyph.code;
					glyph.color2 = ground_glyph.color;
					glyph.coords = { layout.left + x, layout.top + layout.height - 1 - y };
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
			const IVec2 map_cursor{map_x + cursor.x - layout.left, map_y + (layout.top + layout.height - 1 - cursor.y)};
			const uint16_t tile_type_idx = state.selected_tile_type;
			const TerrainType& tile_type = globals.terrain_types[tile_type_idx];
			const int r = state.brush_radius - 1;
			const int d = r * 2 + 1;
			GlyphData glyph;
			glyph.code = tile_type.glyph;
			glyph.color2 = tile_type.color_a;
			glyph.color1 = 0x303030_rgb32;
			glyph.coords = { cursor.x - r, cursor.y - r };
			glyph.size = { d, d };
			render_buffer.push_glyph(elem_id, glyph);

			if (_down)
			{
				if (!state.painting)
				{
					state.painting = true;
					state.paint_cursor = map_cursor;
					paint_square(map, tile_type_idx, map_cursor, state.brush_radius, globals);
				}
				else if (state.paint_cursor != map_cursor)
				{
					paint_line(map, tile_type_idx, state.paint_cursor, map_cursor, state.brush_radius, globals);
					state.paint_cursor = map_cursor;
				}				
			}

			if (!state.dragging_map && _right_down)
			{
				state.dragging_map = true;
				state.dragging_map_coord = map_cursor;
				const auto& ruv = ctx.frame->curr_input.mouse_hit.ruv;
				state.dragging_map_offset = {ruv.x - cursor.x, 1.f - (ruv.y - cursor.y)};
			}
		}
	}

	// TODO: we won't be able to clear this state if we are not rendered
	// have a reliable way from the presenter for initializing and cleaning up
	// states, maybe this should be the internal states of the elements?
	// another way is asking the presenter if this element was rendered last frame
	// or similarly whether an initialization action should run because element
	// wasn't there last frame
	if (!_down)
	{
		state.painting = false;
	}

	return elem_id;
}

static void palette_button(const Context ctx, int index, uint16_t glyph_code, const Color32& color, int val, int& selected_val)
{
	int size = 3;
	int cols = 2;

	int row = (index / cols);
	int col = (index % cols);

	const Id button_id = make_element(ctx, null_id);
	_attr(attrs::placement, ElementPlacement::loose);
	_attr(attrs::left, col * size);
	_attr(attrs::top, row * size);
	_attr(attrs::width, size);
	_attr(attrs::height, size);

	const bool selected = (selected_val == val);

	// TODO: we should make a helper to get both render buffer and layout
	// otherwise user can cache a render_buffer outside of the loop and reuse it
	// while the layout has not been properly finalized
	TabletLayout layout;
	TabletRenderBuffer& render_buffer = access_tablet_render_buffer_and_layout(ctx, layout);
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
	glyph.color1 = 0xd0_rgba32; // darkened translucent background
	glyph.color2 = color;
	draw(glyph_code, {x + 1, y + 1});

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
		selected_val = val;
	}
}

static void tile_palette(const Context ctx, const Array<TerrainType>& terrain_types, EditorState& state)
{
	int size = 3;
	int cols = 2;

	for (int i = 0; i < terrain_types.size(); i++)
	{
		const TerrainType& type_data = terrain_types[i];
		palette_button(_ctx_id(i), i, type_data.glyph, type_data.color_a, i, state.selected_tile_type);
	}
}

static void brush_size_palette(const Context ctx, const Array<TerrainType>& terrain_types, EditorState& state)
{
	int brush_sizes[] = {1, 2, 4, 8};
	int padding = (((int)terrain_types.size() + 1) / 2) * 2; // TODO: hack until we have the proper layout implemented
	for (int i = 0; i < LEN(brush_sizes); i++)
	{
		const int size = brush_sizes[i];
		uint16_t code = ('0' + size);
		palette_button(_ctx_id(i), padding + i, code, 0xffffff_rgb32, size, state.brush_radius);
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

	// TODO: use othographic projection
	Viewpoint vp;
	vp.projection = make_orthographic(vp_width / 2, aspect, 0.f, 100.f);

	viewport(_ctx);
	_attr(attrs::width, window->width());
	_attr(attrs::height, window->height());
	_attr(attrs::viewpoint, vp);
	_attr(attrs::background_color, Color{});

	_children
	{
		// map view
		float max_zoom = 2.0;
		int map_size = (int)(tablet_width * max_zoom) + 4;
		
		if (editor_state.dragging_map)
		{
			if (is_mouse_down(ctx, MouseButtons::right))
			{
				const auto& input = ctx.frame->curr_input;
				DVec2 ndc = calc_ndc({input.mouse_x, input.mouse_y}, window->width(), window->height());
				double world_x = ndc.x * vp_width / 2 - editor_state.dragging_map_offset.x;
				double world_y = ndc.y * vp_height / 2 - editor_state.dragging_map_offset.y;
				int half_size = map_size / 2;
				int tablet_x = std::clamp((int)std::floor(world_x), -half_size, half_size-1);
				int tablet_y = std::clamp((int)std::floor(world_y), -half_size, half_size-1);
				// TODO: since we are changing the map pose here, it will actually invalidate the iuv hit cell done by previous raycast
				// which is stored in the curr_input.mouse_hit. When the later code is using that iuv (like in the map_view())
				// the values there are no longer valid and will have 1 frame delay, so ideally we should calculate the hit 
				// map coordinates right there, store and use it whenever needed consistently
				editor_state.map_pose_offset = {(float)(world_x - tablet_x), (float)(world_y - tablet_y)};
				editor_state.map_vp = (editor_state.dragging_map_coord - IVec2{tablet_x, tablet_y});
			}
			else
			{
				editor_state.dragging_map = false;
			}
		}

		tablet(_ctx);
		
		Pose map_pose;
		map_pose.pos.x = editor_state.map_pose_offset.x;
		map_pose.pos.y = editor_state.map_pose_offset.y;
		map_pose.pos.z = 1.f;
		_attr(attrs::transform, to_mat44(map_pose));
		_attr(attrs::width, map_size);
		_attr(attrs::height, map_size);
		_attr(attrs::texture, atlas_texture);
		_attr(attrs::shader, tablet_shader);
		_attr(attrs::quad_shader, tablet_screen_shader);

		_children
		{
			map_view(_ctx, world.map, globals, editor_state, map_size, map_size);
		}


		// overlay
		tablet(_ctx);
		Pose overlay_tablet_pose;
		overlay_tablet_pose.pos.y = (vp_height - tablet_size.y) / 2;
		_attr(attrs::transform, to_mat44(overlay_tablet_pose));
		_attr(attrs::width, tablet_width);
		_attr(attrs::height, tablet_height);
		_attr(attrs::texture, atlas_texture);
		_attr(attrs::shader, tablet_shader);
		_attr(attrs::quad_shader, tablet_screen_shader);

		_children
		{
			tile_palette(_ctx, globals.terrain_types, editor_state);
			brush_size_palette(_ctx, globals.terrain_types, editor_state);
		}
	}
}

bool Game::ended()
{
	return engine().window->should_close();
}
