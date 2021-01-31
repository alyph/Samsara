#pragma once

#include "globals.h"
#include "map.h"
#include "engine/collection.h"


struct City
{
	Id id{};
	String name = "NEW CITY";
	Vec2i center;
	int32_t development_level[(int)DevelopmentArea::count]{};
	int32_t num_urban_cells{};
	Array<Vec2i> occupied_cells{0, 32};
	Array<Vec2i> dev_tiles{0, 128};
	// TODO: temporary
	Box2i urban_bounds; // including walls
};

struct World
{
	Allocator allocator;
	Map map;
	Collection<City> cities;

	inline void init(Allocator alloc);
};

extern Id create_city(World& world, const Vec2i& center, const Globals& globals);
extern void develop_city(World& world, Id city_id, DevelopmentArea area, const Globals& globals);
static inline bool valid_urban_tile(const Map& map, Vec2i tile_pos, const Globals& globals);
static inline Box2i urban_cells_bounds(const City& city);
static inline Box2i city_cells_bounds(const City& city);

extern void save_world(const World& world, const String& path, const Globals& globals);
extern World load_world(const String& path, const Globals& globals, Allocator alloc);


inline void World::init(Allocator alloc)
{
	allocator = alloc;
	// TODO: maybe should be in Map's constructor
	map.chunks.alloc(0, 1024, alloc);
	map.tiles.alloc(0, 512 * 512, alloc);
	map.ground_glyphs.alloc(0, 512 * 512, alloc);
	cities.alloc(1024, alloc);
}

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



