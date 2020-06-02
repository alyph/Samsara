#pragma once

#include "engine/presenter.h"
#include "engine/texture.h"
#include "world.h"

enum class Brush: int
{
	selection,
	city,
	max,
};

struct EditorState
{
	// TODO: maybe this should be the internal state of the map view
	bool painting{};
	Vec2i paint_cursor;
	int selected_brush{};
	int brush_radius = 1;
	Id selected_city_id{};

	Vec2i map_vp;
	Vec2 map_pose_offset;
	bool dragging_map{};
	Vec2i dragging_map_coord{};
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


