#pragma once

#include "engine/presenter.h"
#include "engine/texture.h"
#include "engine/time.h"
#include "world.h"

enum class Brush: int
{
	selection,
	city,
	max,
};

struct CardPoolEntry
{
	Id card_type{};
	unsigned int weight{};
};

struct GameState
{
	bool in_editor{};
	bool hover_over_map{};
	Vec2i hover_over_map_pos;
	Array<CardPoolEntry> player_card_pool;
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
	Vec2 ref_image_offset;
	Vec2 map_pose_offset;
	bool dragging_map{};
	Vec2i dragging_map_coord{};
	Vec2 dragging_map_offset{};
	float map_scale = 1.f;
};

struct GameRef
{
	GameState& game;
	World& world;
	const Globals& globals;
};

class Game
{
public:
	Game();
	void update(const Time& time);
	bool ended();
	void present(const Context& ctx);
	void shutdown();

private:
	// TODO: managed by asset manager
	Id tablet_shader;
	Id tablet_screen_shader;
	Id atlas_texture;
	Id ref_map_mesh;
	Id ref_map_texture;
	Id ui_texture;

	Globals globals;
	World world;
	GameState game_state;
	EditorState editor_state;

	// double timer = -2.0;
};


