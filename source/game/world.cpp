#include "world.h"
#include "engine/random.h"
#include "engine/serialization.h"
#include "engine/fileio.h"

static constexpr inline auto total_development_level(const City& city)
{
	static_assert((int)DevelopmentArea::count == 2);
	return city.development_level[0] + city.development_level[1];
}

static constexpr inline auto urban_development_level(const City& city)
{
	return city.development_level[(int)DevelopmentArea::urban];
}

static constexpr inline auto rural_development_level(const City& city)
{
	return city.development_level[(int)DevelopmentArea::rural];
}

static void add_development(City& city, Map& map, DevelopmentArea area, const Vec2i& pos, const Globals& globals)
{
	city.development_level[(int)area] += 1;
	size_t total = 0;
	for (int i = 0; i <= (int)area; i++)
	{
		total += city.development_level[i];
	}
	city.visual.development_positions.insert(total-1, pos);
	map.set_structure(pos, globals.development_types[(int)area].structure_type);
}

static void set_wall_tiles(Map& map, const Box2i& wall_bounds, Id wall_type)
{
	const auto& min = wall_bounds.min;
	const auto& max = wall_bounds.max;
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
}

static void setup_walls(City& city, Map& map, Id wall_type, const Globals& globals)
{
	// destroy old walls if there is any
	auto& wall_bounds = city.visual.wall_bounds;
	for (int y = wall_bounds.min.y; y <= wall_bounds.max.y; y++)
	{
		for (int x = wall_bounds.min.x; x <= wall_bounds.max.x; x++)
		{
			Tile* tile = map.get_tile({x, y});
			if (tile && tile->structure && globals.structure_types.get(tile->structure).category == StructureCategory::wall)
			{
				tile->structure = 0;
			}
		}
	}

	// create new walls
	auto min = city.center;
	auto max = min;
	const auto urban_level = urban_development_level(city);
	for (int i = 0; i < urban_level; i++)
	{
		const auto& dev_pos = city.visual.development_positions[i];
		min = comp_min(min, dev_pos);
		max = comp_max(max, dev_pos);
	}

	min = (min - Vec2i{1, 1});
	max = (max + Vec2i{1, 1});
	wall_bounds = {min, max};

	// TODO: we don't want to destroy existing structures, so check and may need to relocate some of the devs
	// for (int x = min.x; x <= max.x; x++)
	// {
	// 	map.set_structure({x, min.y}, wall_type);
	// 	map.set_structure({x, max.y}, wall_type);
	// }

	// for (int y = (min.y + 1); y <= (max.y - 1); y++)
	// {
	// 	map.set_structure({min.x, y}, wall_type);
	// 	map.set_structure({max.x, y}, wall_type);
	// }
	set_wall_tiles(map, wall_bounds, wall_type);

	// TODO: we should fill empty spaces in the wall with a "city ground" structure type
}

Id create_city(World& world, const Vec2i& center, const Globals& globals)
{
	push_perm_allocator(world.allocator);
	City& city = world.cities.emplace();
	city.center = center;
	// TODO: everytime we create new city a new array is allocated
	// ideally we could reuse it as the cities collection is essentially a pool
	// maybe Array should provide a function optionally allocate (only if the handle has not been allocated)
	// TODO: some terrains may be changed when city is placed for example forests will be cleared
	// some terrain may remain e.g. the hilly area
	// some terrain may prevent city from being placed e.g. mountainous, water etc.
	add_development(city, world.map, DevelopmentArea::urban, center, globals);
	city.visual.wall_bounds = to_box(center);
	setup_walls(city, world.map, globals.defines.starting_wall_type, globals);
	world.map.update_glyphs(city.visual.wall_bounds, globals);
	pop_perm_allocator();
	return city.id;
}

inline size_t ring_starting_index(int ring)
{
	return ring ? (ring * (ring - 1) * 4 + 1) : 0;
}

inline size_t to_ring_index(const Vec2i& d)
{
	int ring = std::max(std::abs(d.x), std::abs(d.y));
	if (ring)
	{
		size_t start = ring_starting_index(ring);
		int side{0}, step{0};
		if (d.y == ring)        { side = 0; step = (ring + d.x); }
		else if (d.x == ring)   { side = 1; step = (ring - d.y); }
		else if (d.y == -ring)  { side = 2; step = (ring - d.x); }
		else /*(d.x == -ring)*/ { side = 3; step = (ring + d.y); }
		return step + 2*ring*side + start;
	}
	return 0;
}

inline Vec2i to_ring_tile_pos(const Vec2i& center, int ring, size_t idx)
{
	const size_t start = ring_starting_index(ring);
	const size_t end = ring_starting_index(ring + 1);
	asserts(idx >= start && idx < end);
	if (ring)
	{
		Vec2i d;
		const int idx_in_ring = (int)(idx - start);
		const int steps_per_side = (2 * ring);
		const int side = (idx_in_ring / steps_per_side);
		const int step = (idx_in_ring - (steps_per_side * side));
		switch (side)
		{
			case 0: d.y = ring;  d.x = (step - ring); break;
			case 1: d.x = ring;  d.y = (ring - step); break;
			case 2: d.y = -ring; d.x = (ring - step); break;
			case 3: d.x = -ring; d.y = (step - ring); break;
		}
		return (center + d);
	}
	return center;
}

static Vec2i find_tile_for_dev(const City& city, const Map& map, DevelopmentArea area, const Globals& globals)
{
	if (area == DevelopmentArea::urban)
	{
		const auto urban_level = urban_development_level(city);
		auto occupied = make_temp_array<int>(0, urban_level * 2);
		size_t max_occupied_ring = 0;
		for (int i = 0; i < urban_level; i++)
		{
			const auto ring_idx = to_ring_index(city.visual.development_positions[i] - city.center);
			max_occupied_ring = std::max(ring_idx, max_occupied_ring);
			if (ring_idx >= occupied.size())
			{
				occupied.resize(ring_idx + 1);
			}
			occupied[ring_idx] = 1;
		}

		const int tile_pool_size = 5;
		auto new_tiles = make_temp_array<Vec2i>(0, (max_occupied_ring + 1) *  8);

		int ring = 1;
		size_t idx = ring_starting_index(ring);
		size_t next_ring_idx = ring_starting_index(ring+1);

		while (true)		
		{
			if (idx >= occupied.size() || occupied[idx] == 0)
			{
				new_tiles.push_back(to_ring_tile_pos(city.center, ring, idx));
			}
			if (++idx >= next_ring_idx)
			{
				next_ring_idx = ring_starting_index(++ring + 1);
				if (new_tiles.size() >= tile_pool_size) break;
			}
		}
		return new_tiles[rand_int(new_tiles.size())];
	}
	else
	{
		auto scale_up = [](int x) { return (x >= 0 ? (x + 1) : (x - 1)) / 3; };
		auto scale_down = [](int x) { return x * 3; };
		auto scale_up_vec = [&scale_up](const Vec2i& v) -> Vec2i { return {scale_up(v.x), scale_up(v.y)}; };
		auto scale_down_vec = [&scale_down](const Vec2i& v) -> Vec2i { return {scale_down(v.x), scale_down(v.y)}; };

		const auto& wall_bounds = city.visual.wall_bounds;
		Box2i scaled_bounds;
		const auto r = scaled_bounds.max.x = scale_up(wall_bounds.max.x - city.center.x);
		const auto t = scaled_bounds.max.y = scale_up(wall_bounds.max.y - city.center.y);
		const auto l = scaled_bounds.min.x = scale_up(wall_bounds.min.x - city.center.x);
		const auto b = scaled_bounds.min.y = scale_up(wall_bounds.min.y - city.center.y);

		const int max_wall_ring = std::max({std::abs(scaled_bounds.max.x), std::abs(scaled_bounds.max.y), std::abs(scaled_bounds.min.x), std::abs(scaled_bounds.min.y)});
		const int starting_ring = max_wall_ring + 1;

		const auto urban_level = urban_development_level(city);
		const auto rural_level = rural_development_level(city);
		auto occupied = make_temp_array<int>(0, urban_level * 2);
		size_t max_occupied_ring = 0;
		for (int i = urban_level; i < (urban_level + rural_level); i++)
		{
			const auto ring_idx = to_ring_index(scale_up_vec(city.visual.development_positions[i] - city.center));
			max_occupied_ring = std::max(ring_idx, max_occupied_ring);
			if (ring_idx >= occupied.size())
			{
				occupied.resize(ring_idx + 1);
			}
			occupied[ring_idx] += 1;
		}
		// TODO: road occupation

		const int tile_pool_size = 5;
		auto big_tiles = make_temp_array<Vec2i>(0, (max_occupied_ring + 1) *  24);

		int ring = starting_ring;
		size_t idx = ring_starting_index(ring);
		size_t next_ring_idx = ring_starting_index(ring+1);
		int added_ideal_big_tiles = 0;

		while (true)		
		{
			int num_occupied = idx >= occupied.size() ? 0 : occupied[idx];
			// TODO: check to make sure it has enough usable terrains and other structures
			if (num_occupied <= 2)
			{
				const auto big_tile_rel_pos = to_ring_tile_pos({0, 0}, ring, idx);
				int weight = 1;
				if (num_occupied <= 1)
				{
					weight = 3;
					added_ideal_big_tiles++;
				}
				for (int i = 0; i < weight; i++)
				{
					big_tiles.push_back(big_tile_rel_pos);
				}
			}
			if (++idx >= next_ring_idx)
			{
				next_ring_idx = ring_starting_index(++ring + 1);
				if (added_ideal_big_tiles >= tile_pool_size) break;
			}
		}

		const auto selected_big_tile = big_tiles[rand_int(big_tiles.size())];
		const auto center_tile = city.center + scale_down_vec(selected_big_tile);
		auto new_tiles = make_temp_array<Vec2i>(0, 9);
		for (int y = -1; y <= 1; y++)
		{
			for (int x = -1; x <= 1; x++)
			{
				const auto tile_pos = center_tile + Vec2i{x, y};
				const auto tile = map.get_tile(tile_pos);
				const auto category = tile ? globals.structure_types.get(tile->structure).category : StructureCategory::none;
				if (category != StructureCategory::dev && category != StructureCategory::wall) // TODO: road
				{
					new_tiles.push_back(tile_pos);
				}
			}
		}
		return new_tiles[rand_int(new_tiles.size())];
	}
}

void develop_city(World& world, Id city_id, DevelopmentArea area, const Globals& globals)
{
	auto& city = world.cities.get(city_id);
	auto& map = world.map;
	const auto dev_pos = find_tile_for_dev(city, map, area, globals);
	add_development(city, map, area, dev_pos, globals);
	if (area == DevelopmentArea::urban)
	{
		setup_walls(city, map, globals.defines.starting_wall_type, globals);
		world.map.update_glyphs(city.visual.wall_bounds, globals);
	}
	else
	{
		world.map.update_glyphs(to_box(dev_pos), globals);
	}
}

namespace serialization
{
	struct MapTilesBlob;

	template<class T>
	static void serialize(T& op, RefType<T, City> city)
	{
		op.prop("center", city.center);
		op.prop("dev_urban", city.development_level[(int)DevelopmentArea::urban]);
		op.prop("dev_rural", city.development_level[(int)DevelopmentArea::rural]);
		op.prop("vis_wall", city.visual.wall_bounds);
		op.prop("vis_dev_tiles", city.visual.development_positions);
	}

	template<class T>
	static void serialize(T& op, RefType<T, Map> map)
	{
		op.section("terrains");
		op.blob<MapTilesBlob>(map);
	}

	struct MapTilesBlob
	{
		template<class TOp>
		static inline void write(TOp& op, const Map& map)
		{
			for (int y = 0; y < map.chunk_bounds.height; y++)
			{
				for (int x = 0; x < map.chunk_bounds.width; x++)
				{
					Vec2i chunk_coords{x + map.chunk_bounds.x, y + map.chunk_bounds.y};
					size_t chunk_idx = map.chunk_coords_to_idx(chunk_coords);
					Id first_tile_id = map.chunks[chunk_idx].first_tile_id;
					if (first_tile_id)
					{
						size_t first_tile_idx = id_to_index(first_tile_id);
						bool empty_chunk = true;
						for (size_t idx = 0; idx < (map_chunk_size * map_chunk_size); idx++)
						{
							if (map.tiles[idx + first_tile_idx].terrain != 0)
							{
								empty_chunk = false;
								break;
							}
						}
						
						if (!empty_chunk)
						{
							op.new_object();
							op.value(chunk_coords);
							op.end_heading();

							for (int ty = 0; ty < map_chunk_size; ty++)
							{
								for (int tx = 0; tx < map_chunk_size; tx++)
								{
									const auto terrain_id = map.tiles[first_tile_idx + tx + ty * map_chunk_size].terrain;
									const auto& terrain_type = op.context.globals->terrain_types.get(terrain_id);
									op.blob_char(terrain_type.symbol);
								}
								op.newline();
							}
							op.newline();
						}
					}
				}
			}
		}
		
		template<class TOp>
		static inline void read(TOp& op, Map& map)
		{
			TypeId terrain_map[256]{};
			for (const auto& terrain_type : op.context.globals->terrain_types)
			{
				terrain_map[terrain_type.symbol] = to_type_id(terrain_type.id);
			}
			while (op.next_object())
			{
				Vec2i chunk_coords;
				op.value(chunk_coords);
				op.end_heading();
				// TODO: we shouldn't expand if entire blob is just 0s (terrain type id == 0)
				Id first_tile_id = map.ensure_tiles_for_chunk(chunk_coords);
				size_t first_tile_idx = id_to_index(first_tile_id);

				// read each line until end of the object
				int y = 0;
				while (!op.at_delim(Delimiter::object_tag) &&
					!op.at_delim(Delimiter::section_open) &&
					y < map_chunk_size)
				{
					int x = 0;
					char symbol;
					while (x < map_chunk_size && op.blob_char(symbol))
					{
						map.tiles[first_tile_idx + x + y * map_chunk_size].terrain = terrain_map[symbol];
						x++;
					}
					y++;

					if (!op.blob_next_line())
					{
						break;
					}
				}
			}
			for (size_t ci = 0; ci < map.chunks.size(); ci++)
			{
				if (map.chunks[ci].first_tile_id)
				{
					const auto coords = map.chunk_idx_to_coords(ci);
					const auto bounds = make_box_wh(chunk_first_tile(coords.x), chunk_first_tile(coords.y), map_chunk_size, map_chunk_size);
					map.update_glyphs(bounds, *op.context.globals);
				}
			}
		}
	};

	template<SerializeOpType E>
	static void serialize_world(RefTypeE<E, World> world, const String& path, const Globals& globals)
	{
		struct WorldSerializationContext
		{
			const Globals* globals{};
		}
		context{&globals};

		auto op = make_file_op<E>(format_str("%s/%s", path, "world.dat"), context);
		op.section("cities");
		op.collection(world.cities, "city");

		op = make_file_op<E>(format_str("%s/%s", path, "map.dat"), context);
		serialize(op, world.map);
	}
}



void save_world(const World& world, const String& path, const Globals& globals)
{
	serialization::serialize_world<SerializeOpType::write>(world, path, globals);
}

World load_world(const String& path, const Globals& globals, Allocator alloc)
{
	push_perm_allocator(alloc);
	World world;
	world.init(alloc);
	serialization::serialize_world<SerializeOpType::read>(world, path, globals);
	for (const auto& city : world.cities)
	{
		const auto dev_level = total_development_level(city);
		int start = 0;
		for (int area = 0; area < (int)DevelopmentArea::count; area++)
		{
			const auto dev_level = city.development_level[area];
			const auto& dev_type = globals.development_types[area];
			for (int i = start; i < start + dev_level; i++)
			{
				const auto dev_pos = city.visual.development_positions[i];
				world.map.set_structure(dev_pos, dev_type.structure_type);
				world.map.update_glyphs(to_box(dev_pos), globals);
			}
			start += dev_level;
		}

		set_wall_tiles(world.map, city.visual.wall_bounds, globals.defines.starting_wall_type); // TODO: store wall type of the city
		world.map.update_glyphs(city.visual.wall_bounds, globals);
	}
	pop_perm_allocator();
	return world;
}


