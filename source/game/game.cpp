#include "game.h"
#include "engine/window.h"
#include "engine/viewport.h"
#include "engine/tablet.h"
#include "engine/shader.h"
#include "engine/image_utils.h"
#include "engine/mesh.h"
#include "engine/filesystem.h"
#include "easy/profiler.h"

static constexpr const Vec2i initial_map_vp{ (map_chunk_size / 2), (map_chunk_size / 2) };
static const String main_scenario_dir = "../../data/game/scenarios/main";

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
		// "../../data/fonts/anikki_square_24x24.png",
		"../../data/fonts/nice_curses_24x24.png",
		"../../data/fonts/urr_a_24x24.png",
		"../../data/fonts/path_24x24.png",
	});

	desc.vs_path = "../../data/shaders/mesh_vs.gls";
	desc.fs_path = "../../data/shaders/mesh_fs.gls";
	const auto ref_map_shader = create_shader(desc);

	// TODO: just one pixel per tile, will adjust it later if needed
	const float half_w = 3648 / 2; // note: integer
	const float half_h = 3004 / 2;
	const Color mesh_tint{ 1.f, 1.f, 1.f, 0.3f };
	Mesh mesh;
	mesh.indices = { 0, 1, 2, 0, 2, 3 };
	mesh.vertices = 
	{
		{ -half_w, -half_h, 0 }, { half_w, -half_h, 0 }, 
		{ half_w, half_h, 0 }, { -half_w, half_h, 0 },
	};
	mesh.colors = 
	{
		mesh_tint, mesh_tint, mesh_tint, mesh_tint, 
	};
	mesh.uvs =
	{
		{ 0.f, 0.f }, { 1.f, 0.f }, { 1.f, 1.f }, { 0.f, 1.f },
	};
	ref_map_mesh = add_mesh(std::move(mesh), ref_map_shader);
	ref_map_texture = load_texture("../../data/images/europe1054ref.png");

	const auto make_dev_type = [&](const String& dev_key, const String& struct_key, const String& name, DevelopmentArea area, uint16_t glyph, const Color32& color) -> DevelopmentType&
	{
		auto& dev = globals.development_types.set(dev_key, {0, name, area, 0});
		auto& structure = globals.structure_types.set(struct_key, {0, name, glyph, color, StructureCategory::dev, dev.id});
		dev.structure_type = structure.id;
		return dev;
	};

	const auto alloc = perm_allocator();

	globals.terrain_types.alloc(64, alloc);
	globals.terrain_types.set(0, "clear", {0, "clear", '_', 0, 0_rgb32, 0_rgb32});
	globals.terrain_types.set("forest", {0, "forest", 'T', 0x0105, 0x30ff50_rgb32, 0x30ff50_rgb32});
	globals.terrain_types.set("hill", {0, "hill", '^', 0x0218, 0x606070_rgb32, 0x606070_rgb32});
	globals.terrain_types.set("coast", {0, "coast", '=', 0x0300, 0x103060_rgb32, 0x80c0f0_rgb32, TileTypeFlags::water});

	globals.structure_types.alloc(1024, alloc);
	globals.structure_types.set(0, "none", {0, "none", 0, 0_rgb32, StructureCategory::none, 0});

	globals.development_types.alloc(512, alloc);
	const auto& governing_dev = make_dev_type("gov", "d_gov", "governing", DevelopmentArea::urban, 0x007f, 0xe000e0_rgb32);
	make_dev_type("comm", "d_comm", "commercial", DevelopmentArea::urban, 0x007f, 0xf0f000_rgb32);
	make_dev_type("ind", "d_ind", "industrial", DevelopmentArea::urban, 0x007f, 0x0030f0_rgb32);
	make_dev_type("farm", "d_farm", "farming", DevelopmentArea::rural, 0x007f, 0x40ef20_rgb32);

	const auto& wall_struct = globals.structure_types.set("wall", {0, "wall", 0x0320, 0xafafaf_rgb32, StructureCategory::wall, 0});

	globals.defines.starting_dev_type = governing_dev.id;
	globals.defines.starting_wall_type = wall_struct.id;

	editor_state.selected_brush = 1;
	editor_state.map_vp = initial_map_vp;
	
	if (filesystem::exists(main_scenario_dir))
	{
		world = load_world(main_scenario_dir, globals, stage_allocator());
	}
	else
	{
		world.init(stage_allocator());
	}
}


void Game::update()
{
}



static void paint_square(Map& map, Id terrain_type, const Vec2i& center, int radius, const Globals& globals)
{
	// TODO: move this function to map and let it do less glyph update
	int r = (radius - 1);
	for (int dx = -r; dx <= r; dx++)
	{
		for (int dy = -r; dy <= r; dy++)
		{
			map.set_terrain(center + Vec2i{dx, dy}, terrain_type);
		}
	}
	map.update_glyphs({center - Vec2i{r, r}, center + Vec2i{r, r}}, globals);
}

static void paint_line(Map& map, Id terrain_type, const Vec2i& start, const Vec2i& end, int radius, const Globals& globals)
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
	paint_square(map, terrain_type, {x, y}, radius, globals);
	while (!((x == end.x && y == end.y)))
	{
		const auto e2 = err * 2;
		if (e2 >= dy) { err += dy; x += sx; }
		if (e2 <= dx) { err += dx; y += sy; }
		// TODO: if this gets too expensive, can just draw the deltas
		paint_square(map, terrain_type, {x, y}, radius, globals);
	}
}

static const constexpr uint16_t sel_brush_glyph = 0x02f1;
static const constexpr Color32 sel_brush_color = 0xffffff_rgb32;
static const constexpr uint16_t city_brush_glyph = 0x02e1;
static const constexpr Color32 city_brush_color = 0xffff00_rgb32;

static inline Vec2i to_tablet_coords(const Vec2i& map_coords, const TabletLayout& layout, const Vec2i& map_min_coords)
{
	return { layout.left + map_coords.x - map_min_coords.x, layout.top + layout.height - 1 - (map_coords.y - map_min_coords.y) };
}

static Id map_view(const Context ctx, World& world, const Globals& globals, EditorState& state, const Scalar& width, const Scalar& height)
{
	EASY_FUNCTION();

	const Id elem_id = make_element(ctx, null_id);
	_attr(attrs::width, width);
	_attr(attrs::height, height);

	TabletLayout layout;
	TabletRenderBuffer& render_buffer = access_tablet_render_buffer_and_layout(ctx, layout);
	int map_x = state.map_vp.x - layout.width / 2;
	int map_y = state.map_vp.y - layout.height / 2;
	const Vec2i map_min{map_x, map_y};

	Map& map = world.map;
	for (int y = 0; y < layout.height; y++)
	{
		for (int x = 0; x < layout.width; x++)
		{		
			Vec2i tile_coords{ map_x + x, map_y + y };
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

	if (state.selected_city_id)
	{
		const auto& city = world.cities.get(state.selected_city_id);
		GlyphData glyph;
		glyph.code = 0;
		glyph.color1 = 0xffffff50_rgba32;
		glyph.coords = to_tablet_coords({city.wall_bounds.min.x, city.wall_bounds.max.y}, layout, map_min);
		glyph.size = {city.wall_bounds.max.x - city.wall_bounds.min.x + 1, city.wall_bounds.max.y - city.wall_bounds.min.y + 1};
		render_buffer.push_glyph(elem_id, glyph);
	}

	if (_hover)
	{
		const Vec2i& cursor = ctx.frame->curr_input.mouse_hit.iuv;
		if (cursor.x >= layout.left && cursor.x < (layout.left + layout.width) &&
			cursor.y >= layout.top && cursor.y < (layout.top + layout.height))
		{
			const Vec2i map_cursor{map_x + cursor.x - layout.left, map_y + (layout.top + layout.height - 1 - cursor.y)};

			if (state.selected_brush == (int)Brush::selection)
			{
				if (_pressed)
				{
					for (const auto& city : world.cities)
					{
						if (encompasses(city.wall_bounds, map_cursor))
						{
							state.selected_city_id = city.id;
							break;
						}
					}
				}
			}
			else
			{
				uint16_t code{};
				Color32 color{};
				int r{};

				int terrain_brush = (state.selected_brush - (int)Brush::max);
				if (terrain_brush >= 0 /* && terrain_brush < globals.terrain_types.size()*/)
				{
					const TerrainType& tile_type = globals.terrain_types.get(terrain_brush);
					code = tile_type.glyph;
					color = tile_type.color_a;
					r = state.brush_radius - 1;

					if (_down)
					{
						if (!state.painting)
						{
							state.painting = true;
							state.paint_cursor = map_cursor;
							paint_square(map, terrain_brush, map_cursor, state.brush_radius, globals);
						}
						else if (state.paint_cursor != map_cursor)
						{
							paint_line(map, terrain_brush, state.paint_cursor, map_cursor, state.brush_radius, globals);
							state.paint_cursor = map_cursor;
						}				
					}
				}
				else if (state.selected_brush == (int)Brush::city)
				{
					// city
					code = city_brush_glyph;
					color = city_brush_color;

					if (_pressed)
					{
						create_city(world, map_cursor, globals);
					}
				}

				const int d = r * 2 + 1;
				GlyphData glyph;
				glyph.code = code;
				glyph.color2 = color;
				glyph.color1 = 0x303030_rgb32;
				glyph.coords = { cursor.x - r, cursor.y - r };
				glyph.size = { d, d };
				render_buffer.push_glyph(elem_id, glyph);
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

static int palette_button(const Context ctx, int index, int begin_row, uint16_t glyph_code, const Color32& color, int val, int& selected_val)
{
	int size = 3;
	int cols = 2;

	int row = begin_row + (index / cols);
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

	auto draw = [&](uint16_t code, const Vec2i& coords)
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

	return row;
}

static void tile_palette(const Context ctx, const Collection<TerrainType>& terrain_types, EditorState& state, int& row)
{
	int begin_row = row;
	int idx = 0;
	for (const auto& type : terrain_types)
	{
		row = palette_button(_ctx_id(idx), idx, begin_row, type.glyph, type.color_a, (int)Brush::max + (int)type.id, state.selected_brush);
		idx++;
	}
}

static void brush_size_palette(const Context ctx, EditorState& state, int& row)
{
	int begin_row = row;
	int brush_sizes[] = {1, 2, 4, 8};
	for (int i = 0; i < LEN(brush_sizes); i++)
	{
		const int size = brush_sizes[i];
		uint16_t code = ('0' + size);
		row = palette_button(_ctx_id(i), i, begin_row, code, 0xffffff_rgb32, size, state.brush_radius);
	}
}

static void city_dev_palette(const Context ctx, Id sel_city, const Globals& globals, World& world, int& row)
{
	int begin_row = row;
	int sel_dev_type = 0;
	int idx = 0;
	for (const auto& dev_type : globals.development_types)
	{
		const StructureType& struct_type = globals.structure_types.get(dev_type.structure_type);
		row = palette_button(_ctx_id(idx), idx, begin_row, struct_type.glyph, struct_type.color, (int)dev_type.id, sel_dev_type);
		idx++;
	}

	if (sel_dev_type > 0)
	{
		develop_city(world, sel_city, sel_dev_type, globals);
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

	Viewpoint vp;
	vp.projection = make_orthographic(vp_width / 2, aspect, 0.f, 100.f);

	viewport(_ctx);
	_attr(attrs::width, window->width());
	_attr(attrs::height, window->height());
	_attr(attrs::viewpoint, vp);
	_attr(attrs::background_color, (Color{0.f, 0.f, 0.f, 1.f}));

	_children
	{
		// map view
		float min_zoom = 0.25f;
		float max_zoom = 2.0;
		int map_size = (int)(tablet_width * max_zoom) + 4;
		editor_state.map_scale += _mouse_wheel_delta * 0.001f;
		const auto map_scale = editor_state.map_scale = std::clamp(editor_state.map_scale, 1/max_zoom, 1/min_zoom);
		
		// TODO: code below and serveral other places will not work if the map cells are not square
		// it should employ a more generalized algorithem:
		// 1. calculate the new map tablet position based on the new mouse position and real number cell coordinates (ruv?) it's dragging on 
		// 2. offset this new map position with the new map_vp, so the map position never goes over one cell
		// 3. other overlay images (like the ref map) should follow the map position calculated in step 1
		if (editor_state.dragging_map)
		{
			if (is_mouse_down(ctx, MouseButtons::right))
			{
				const auto& input = ctx.frame->curr_input;
				Vec2d ndc = calc_ndc({input.mouse_x, input.mouse_y}, window->width(), window->height());
				double world_x = ndc.x * vp_width / (2 * map_scale) - editor_state.dragging_map_offset.x;
				double world_y = ndc.y * vp_height / (2 * map_scale) - editor_state.dragging_map_offset.y;
				int half_size = map_size / 2;
				int tablet_x = std::clamp((int)std::floor(world_x), -half_size, half_size-1);
				int tablet_y = std::clamp((int)std::floor(world_y), -half_size, half_size-1);
				// TODO: since we are changing the map pose here, it will actually invalidate the iuv hit cell done by previous raycast
				// which is stored in the curr_input.mouse_hit. When the later code is using that iuv (like in the map_view())
				// the values there are no longer valid and will have 1 frame delay, so ideally we should calculate the hit 
				// map coordinates right there, store and use it whenever needed consistently
				editor_state.map_pose_offset = {(float)(world_x - tablet_x), (float)(world_y - tablet_y)};
				editor_state.map_vp = (editor_state.dragging_map_coord - Vec2i{tablet_x, tablet_y});
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
		const auto map_scale_mat = make_mat44_scale({map_scale, map_scale, 1.f});
		_attr(attrs::transform, map_scale_mat * to_mat44(map_pose));
		_attr(attrs::width, map_size);
		_attr(attrs::height, map_size);
		_attr(attrs::texture, atlas_texture);
		_attr(attrs::shader, tablet_shader);
		_attr(attrs::quad_shader, tablet_screen_shader);

		_children
		{
			map_view(_ctx, world, globals, editor_state, map_size, map_size);
		}


		// reference image
		mesh(_ctx);
		Pose mesh_pose;
		mesh_pose.pos = to_vec3(editor_state.map_pose_offset - to_vec2(editor_state.map_vp - initial_map_vp), 0.5);
		_attr(attrs::mesh_id, ref_map_mesh);
		_attr(attrs::transform, map_scale_mat * to_mat44(mesh_pose));
		_attr(attrs::texture, ref_map_texture);


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
			int row = 0;
			tile_palette(_ctx, globals.terrain_types, editor_state, row); 
			row++;
			brush_size_palette(_ctx, editor_state, row);
			row++;
			palette_button(_ctx, 0, row, sel_brush_glyph, sel_brush_color, (int)Brush::selection, editor_state.selected_brush);
			row = palette_button(_ctx, 1, row, city_brush_glyph, city_brush_color, (int)Brush::city, editor_state.selected_brush) + 1;
			if (editor_state.selected_city_id)
			{
				row++;
				city_dev_palette(_ctx, editor_state.selected_city_id, globals, world, row);
				row++;
			}
		}
	}
}

bool Game::ended()
{
	return engine().window->should_close();
}

void Game::shutdown()
{
	filesystem::create_directories(main_scenario_dir);
	save_world(world, main_scenario_dir, globals);
}

