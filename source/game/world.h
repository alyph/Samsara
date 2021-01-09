#pragma once

#include "globals.h"
#include "map.h"
#include "engine/collection.h"

struct CityVisual
{
	Box2i wall_bounds;
	Array<Vec2i> development_positions{0, 128};
};

struct City
{
	Id id;
	Vec2i center;
	int32_t development_level[(int)DevelopmentArea::count]{};

	CityVisual visual;
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



