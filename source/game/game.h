#pragma once

#include "engine/presenter.h"
#include "engine/texture.h"
#include "world.h"

struct EditorState
{
	// TODO: maybe this should be the internal state of the map view
	bool painting{};
	IVec2 paint_cursor;
	int selected_tile_type{};
	int brush_radius = 1;

	IVec2 map_vp;
	Vec2 map_pose_offset;
	bool dragging_map{};
	IVec2 dragging_map_coord{};
	Vec2 dragging_map_offset{};
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

	Globals globals;
	World world;
	EditorState editor_state;
};


