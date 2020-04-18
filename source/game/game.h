#pragma once

#include "engine/presenter.h"
#include "engine/texture.h"
#include "map.h"


struct EditorState
{
	IVec2 map_vp;
	// TODO: maybe this should be the internal state of the map view
	bool painting{};
	IVec2 paint_cursor;
	int selected_tile_type{};
	int brush_radius = 1;
};

struct WorldMeta
{
	std::vector<TileType> tile_types;
	std::vector<uint16_t> ex_tile_glyphes;
};

class Game
{
public:
	Game();
	void update();
	bool ended();
	void present(const Context& ctx);

private:
	// TODO: managed by asset manager
	Id tablet_shader;
	Id tablet_screen_shader;
	Id atlas_texture;

	WorldMeta world_meta;
	Map map;
	EditorState editor_state;
};


