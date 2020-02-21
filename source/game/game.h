#pragma once

#include "engine/presenter.h"
#include "engine/texture.h"
#include "map.h"


struct EditorState
{
	uint16_t selected_tile_type{};
	IVec2 map_vp;
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


