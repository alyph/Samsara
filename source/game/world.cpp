#include "world.h"


static void add_development(City& city, Map& map, TypeIndex dev_type, const Vec2i& coords, const Globals& globals)
{
	city.devs.push_back({dev_type, coords});	
	map.set_structure(coords, globals.development_types[dev_type].structure_type);	
}

static void setup_walls(City& city, Map& map, TypeIndex wall_type, const Globals& globals)
{
	auto min = city.coords;
	auto max = min;
	for (const auto& dev : city.devs)
	{
		const auto& dev_type = globals.development_types[dev.type];
		if (dev_type.area == DevelopmentArea::urban)
		{
			min = comp_min(min, dev.coords);
			max = comp_max(max, dev.coords);
		}
	}

	min = (min - Vec2i{1, 1});
	max = (max + Vec2i{1, 1});

	for (int x = min.x; x <= max.x; x++)
	{
		map.set_structure({x, min.y}, wall_type);
		map.set_structure({x, max.y}, wall_type);
	}

	for (int y = (min.y + 1); y <= (max.y - 1); y++)
	{
		map.set_structure({min.x, y}, wall_type);
		map.set_structure({max.x, y}, wall_type);
	}

	city.wall_bounds = {min, max};
}

Id create_city(World& world, const Vec2i& coords, const Globals& globals)
{
	City& city = world.cities.emplace();
	city.coords = coords;
	// TODO: everytime we create new city a new array is allocated
	// ideally we could reuse it as the cities collection is essentially a pool
	// maybe Array should provide a function optionally allocate (only if the handle has not been allocated)
	// TODO: some terrains may be changed when city is placed for example forests will be cleared
	// some terrain may remain e.g. the hilly area
	// some terrain may prevent city from being placed e.g. mountainous, water etc.
	city.devs.alloc(0, 128, world.allocator);
	add_development(city, world.map, globals.city_rules.starting_dev_type, coords, globals);
	setup_walls(city, world.map, globals.city_rules.starting_wall_type, globals);
	world.map.update_glyphs(city.wall_bounds, globals);
	return city.id;
}

