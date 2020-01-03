#include "game.h"
#include "engine/window.h"
#include "engine/viewport.h"
#include "engine/tablet.h"
#include "engine/shader.h"
#include "engine/image_utils.h"

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

	auto tex_desc = load_texture("../../data/fonts/Anikki_square_32x32.png");
	atlas_texture = Texture::create(tex_desc);

	// tile_types.alloc_stored(0, 64);
	tile_types.push_back({"empty", 0, 0_rgb, 0_rgb});
	tile_types.push_back({"forest", 0x05, 0x30ff50_rgb, 0x60b010_rgb});

	map_vp.x = map_vp.y = (map_chunk_size / 2);
	map.chunks.alloc_stored(0, 1024);
	map.tiles.alloc_stored(0, 512 * 512);
	map.set_tile({0, 0}, {1});
	map.set_tile({16, 16}, {1});
	map.set_tile({10, 10}, {1});
}


void Game::update()
{
}

static Id map_view(const Context ctx, Map& map, const std::vector<TileType>& tile_types, const IVec2& viewpoint, const Scalar& width, const Scalar& height)
{
	const Id elem_id = make_element(ctx, null_id);
	_attr(attrs::width, width);
	_attr(attrs::height, height);

	TabletRenderBuffer& render_buffer = access_tablet_render_buffer(ctx);
	const TabletLayout& layout = get_elem_attr_or_assert(*ctx.frame, elem_id, attrs::tablet_layout);
	int map_x = viewpoint.x - layout.width / 2;
	int map_y = viewpoint.y - layout.height / 2;

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
					const TileType& tile_type = tile_types[tile.type];
					GlyphData glyph;
					glyph.code = (uint8_t)(tile_type.glyph);
					glyph.color2 = to_color32(tile_type.color_a);
					glyph.coords = { layout.left + x, layout.top + y };
					render_buffer.push_glyph(elem_id, glyph);
				}
			}
		}
	}
	return elem_id;
}

void Game::present(const Context& ctx)
{
	using namespace elem;

	auto window = engine().window;

	const float aspect = window->aspect();

	int tablet_width = 100;
	int tablet_height = 64;
	const Vec2 tablet_size = calc_tablet_size(tablet_width, tablet_height, atlas_texture.id());

	const float vp_width = (float)tablet_width;
	const float vp_height = vp_width / aspect;

	// tablet_height = (int)std::floor(vp_height / (tablet_size.y / tablet_height));

	// TODO: use othographic projection
	Viewpoint vp;
	vp.projection = make_orthographic(vp_width / 2, aspect, 0.f, 100.f);
	vp.pose.pos.y = 0.f; //((float)tablet_size.y - vp_height) / 2;

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
		_attr(attrs::texture, atlas_texture.id());
		_attr(attrs::shader, tablet_shader);
		_attr(attrs::quad_shader, tablet_screen_shader);

		_children
		{
			map_view(_ctx, map, tile_types, map_vp, tablet_width, tablet_height);
		}
	}
}

bool Game::ended()
{
	return engine().window->should_close();
}
