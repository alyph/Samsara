#pragma once

#include "globals.h"
#include "map.h"
#include "engine/collection.h"

struct Development
{
	TypeIndex type;
	Vec2i coords;
};

struct City
{
	Id id;
	Vec2i coords;
	Box2i wall_bounds;
	Array<Development> devs;
};

struct World
{
	Allocator allocator;
	Map map;
	Collection<City> cities;

	inline void init(Allocator alloc);
};

extern Id create_city(World& world, const Vec2i& coords, const Globals& globals);


inline void World::init(Allocator alloc)
{
	allocator = alloc;
	// TODO: maybe should be in Map's constructor
	map.chunks.alloc(0, 1024, alloc);
	map.tiles.alloc(0, 512 * 512, alloc);
	map.ground_glyphs.alloc(0, 512 * 512, alloc);
	cities.alloc(1024, alloc);
}



