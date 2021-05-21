#pragma once

#include "globals.h"
#include "map.h"
#include "card.h"


struct City
{
	Id id{};
	String name = "NEW CITY";
	Vec2i center;
	int32_t development_level[(int)DevelopmentArea::count]{};
	int32_t num_urban_cells{};
	Array<Vec2i> occupied_cells;
	Array<Vec2i> dev_tiles;
	// TODO: temporary
	Box2i urban_bounds; // including walls

	inline void alloc(Allocator allocator)
	{
		occupied_cells.alloc(0, 32, allocator);
		dev_tiles.alloc(0, 128, allocator);
	}
};

enum class TurnPhase
{
	none,
	draft,
	action,
	resolution,
};

struct CardAction
{
	CardId card_id{};
};

struct Player
{
	CardPile hand;
	CardPile regular_deck;
	Array<CardId> ephemeral_cards;
	Array<CardAction> actions;
	int action_size = 3;

	inline void alloc(Allocator allocator)
	{
		hand.alloc(0, 32, allocator);
		regular_deck.alloc(0, 64, allocator);
		ephemeral_cards.alloc(0, 8, allocator);
		actions.alloc(0, 16, allocator);
	}
};

struct World
{
	Allocator allocator;

	int64_t turn = -1;
	TurnPhase phase = TurnPhase::none;
	int starting_year = 0;

	Map map;
	Collection<City> cities;

	Player player;

	inline void alloc(Allocator allocator)
	{
		this->allocator = allocator;
		map.alloc(allocator);
		cities.alloc(1024, allocator);
		player.alloc(allocator);
	}
};

struct WorldRef
{
	World& world;
	const Globals& globals;
};

struct CityRef
{
	City& city;
	World& world;
	const Globals& globals;
};

static inline CityRef make_city_ref(const WorldRef& world_ref, Id city_id) { return { world_ref.world.cities.get(city_id), world_ref.world, world_ref.globals }; }

extern Id create_city(World& world, const Vec2i& center, const Globals& globals);
extern void develop_city(const WorldRef& ref, Id city_id, DevelopmentArea area, int level);
extern void reset_city_development(const WorldRef& ref, Id city_id, DevelopmentArea area, int level);
static inline bool valid_urban_tile(const Map& map, Vec2i tile_pos, const Globals& globals);
static inline Box2i urban_cells_bounds(const City& city);
static inline Box2i city_cells_bounds(const City& city);

extern void save_world(const World& world, const String& path, const Globals& globals);
extern World load_world(const String& path, const Globals& globals, Allocator alloc);

static inline bool valid_urban_tile(const Map& map, Vec2i tile_pos, const Globals& globals)
{
	for (const auto offset : surrounding_offsets)
	{
		const auto& tile = map.get_tile_or_empty(tile_pos + offset);
		if (globals.terrain_types.get(tile.terrain).flags & TileTypeFlags::water)
		{
			return false;
		}
	}
	return true;
}

static inline Box2i urban_cells_bounds(const City& city)
{
	const auto center_cell = tile_to_cell_coords(city.center);
	auto bounds = to_box(center_cell);
	for (int i = 0; i < city.num_urban_cells; i++)
	{
		expand_box(bounds, city.occupied_cells[i]);
	}
	return bounds;
}

static inline Box2i city_cells_bounds(const City& city)
{
	const auto center_cell = tile_to_cell_coords(city.center);
	auto bounds = to_box(center_cell);
	for (const auto cell_pos : city.occupied_cells)
	{
		expand_box(bounds, cell_pos);
	}
	return bounds;
}



