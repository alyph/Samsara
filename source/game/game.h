#pragma once

#include "engine/presenter.h"
#include "engine/texture.h"
#include "map.h"

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
	Texture atlas_texture;

	Map map;
	std::vector<TileType> tile_types;
	IVec2 map_vp;
};


