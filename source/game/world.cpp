#include "world.h"
#include "game_utils.h"
#include "engine/random.h"
#include "engine/serialization.h"
#include "engine/fileio.h"

struct UrbanMask
{
	uint8_t should_fill[256];
	constexpr UrbanMask(): should_fill{}
	{
		constexpr const uint8_t corners = (0x02 | 0x08 | 0x20 | 0x80);
		for (unsigned int i = 0; i < 256; i++)
		{
			uint8_t mask = (uint8_t)i;
			if (mask & 0x01) { mask |= (0x80 | 0x02); }
			if (mask & 0x04) { mask |= (0x02 | 0x08); }
			if (mask & 0x10) { mask |= (0x08 | 0x20); }
			if (mask & 0x40) { mask |= (0x20 | 0x80); }			
			should_fill[i] = (mask & corners) == corners ? 1 : 0;
		}
	}
};
static const constexpr UrbanMask urban_mask{};

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

static inline bool is_urban_tile(const Map& map, Vec2i tile_pos, const Globals& globals)
{
	const auto& tile = map.get_tile_or_empty(tile_pos);
	return (tile.structure && has_all(globals.structure_types.get(tile.structure).flags, StructureFlags::urban));
}

static inline Box2i urban_tile_bounds(City& city, const Map& map, const Globals& globals)
{
	auto bounds = to_box(city.center);
	for (int i = 0; i < city.num_urban_cells; i++)
	{
		const auto tile_top_left = cell_to_tile_coords(city.occupied_cells[i]);
		for (int ti = 0; ti < map_cell_size * map_cell_size; ti++)
		{
			const auto tile_pos = tile_top_left + Vec2i{ti % map_cell_size, ti / map_cell_size};
			if (is_urban_tile(map, tile_pos, globals))
			{
				expand_box(bounds, tile_pos);
			}
		}
	}
	pad_box(bounds, 1); // pad 1 to include walls
	return bounds;
}

static void add_development(City& city, Map& map, DevelopmentArea area, const Vec2i& pos, const Globals& globals)
{
	city.development_level[(int)area] += 1;
	size_t total = 0;
	for (int i = 0; i <= (int)area; i++)
	{
		total += city.development_level[i];
	}
	city.dev_tiles.insert(total-1, pos);
	map.set_structure(pos, globals.development_types[(int)area].structure_type);
}

#if 0
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
			if (tile && tile->structure && has_all(globals.structure_types.get(tile->structure).flags, StructureFlags::wall))
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
#endif

static void setup_walls(City& city, Map& map, const Globals& globals)
{
	for (int i = 0; i < city.num_urban_cells; i++)
	{
		const auto cell_pos = city.occupied_cells[i];
		const auto top_left_tile = cell_to_tile_coords(cell_pos);
		Box2i bounds;
		bool has_walls = false;
		for (int y = -1; y < map_cell_size + 2; y++)
		{
			for (int x = -1; x < map_cell_size + 2; x++)
			{
				const auto tile_pos = top_left_tile + Vec2i{x, y};
				if (!is_urban_tile(map, tile_pos, globals))
				{
					for (const auto offset : surrounding_offsets)
					{
						if (is_urban_tile(map, tile_pos + offset, globals))
						{
							map.set_structure(tile_pos, globals.defines.starting_wall_type); // TODO: use city wall type
							if (has_walls)
							{
								expand_box(bounds, tile_pos);
							}
							else
							{
								bounds = to_box(tile_pos);
								has_walls = true;
							}
							break;
						}
					}

				}
			}
		}
		if (has_walls)
		{
			map.update_glyphs(bounds, globals);
		}
	}
}

Id create_city(World& world, const Vec2i& center, const Globals& globals)
{
	scoped_context_allocator(world.allocator);
	City& city = world.cities.emplace();
	city.center = center;
	city.num_urban_cells = 1;
	city.occupied_cells.push_back(tile_to_cell_coords(center));
	// TODO: everytime we create new city a new array is allocated
	// ideally we could reuse it as the cities collection is essentially a pool
	// maybe Array should provide a function optionally allocate (only if the handle has not been allocated)
	// TODO: some terrains may be changed when city is placed for example forests will be cleared
	// some terrain may remain e.g. the hilly area
	// some terrain may prevent city from being placed e.g. mountainous, water etc.
	add_development(city, world.map, DevelopmentArea::urban, center, globals);
	setup_walls(city, world.map, globals);
	city.urban_bounds = urban_tile_bounds(city, world.map, globals);
	return city.id;
}

#if 0
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
	auto scale_up = [](int x) { return (x >= 0 ? (x + 1) : (x - 1)) / 3; };
	auto scale_down = [](int x) { return x * 3; };
	auto scale_up_vec = [&scale_up](const Vec2i& v) -> Vec2i { return {scale_up(v.x), scale_up(v.y)}; };
	auto scale_down_vec = [&scale_down](const Vec2i& v) -> Vec2i { return {scale_down(v.x), scale_down(v.y)}; };

	const auto& wall_bounds = city.urban_bounds;
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
		const auto ring_idx = to_ring_index(scale_up_vec(city.dev_tiles[i] - city.center));
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
			const auto flags = tile ? globals.structure_types.get(tile->structure).flags : StructureFlags::none;
			if (!has_any(flags, StructureFlags::dev | StructureFlags::wall)) // TODO: road
			{
				new_tiles.push_back(tile_pos);
			}
		}
	}
	return new_tiles[rand_int(new_tiles.size())];
}
#endif

static Vec2i make_rural_space(City& city, Map& map, const Globals& globals)
{
	const auto bounds = city_cells_bounds(city);
	const auto bounds_size = box_size(bounds);
	const auto rel_bounds = make_box_wh(0, 0, bounds_size.x, bounds_size.y);
	auto city_cells_2d = make_temp_array<uint8_t>(bounds_size.x * bounds_size.y);
	const auto center_cell = tile_to_cell_coords(city.center);
	const auto center_cell_rel = center_cell - bounds.min;
	const auto num_city_cells = city.occupied_cells.size();

	int max_expansion_dist = 0;
	for (int i = 0; i < city.occupied_cells.size(); i++)
	{
		const auto cell_pos = city.occupied_cells[i];
		const auto rel_pos = cell_pos - bounds.min;
		const int idx = (rel_pos.y * bounds_size.x + rel_pos.x);
		city_cells_2d[idx] = 1 + (uint8_t)(i < city.num_urban_cells ? DevelopmentArea::urban : DevelopmentArea::rural);
		max_expansion_dist = std::max(max_expansion_dist, manhattan_length(rel_pos - center_cell_rel));
	}
	max_expansion_dist += 1; // max distance from potential expansion cell to the center

	auto occupied = [&rel_bounds, &bounds_size, &city_cells_2d](const Vec2i& pos)
	{
		return encompasses(rel_bounds, pos) && (city_cells_2d[pos.y * bounds_size.x + pos.x] > 0);
	};
	auto urban_occupied = [&rel_bounds, &bounds_size, &city_cells_2d](const Vec2i& pos)
	{
		return encompasses(rel_bounds, pos) && (city_cells_2d[pos.y * bounds_size.x + pos.x] == (1 + (uint8_t)DevelopmentArea::urban));
	};
	auto rural_occupied = [&rel_bounds, &bounds_size, &city_cells_2d](const Vec2i& pos)
	{
		return encompasses(rel_bounds, pos) && (city_cells_2d[pos.y * bounds_size.x + pos.x] == (1 + (uint8_t)DevelopmentArea::rural));
	};

	auto calc_rural_adjacency = [&map, &globals](Vec2i tile_pos)
	{
		// NOTE: when walls push into the the adjacent spots of the rural dev, we may not relocate it, hence it may be temporarily invalid, which is probably OK
		return count_adjacent_structures_with_any_flag(map, tile_pos, StructureFlags::rural|StructureFlags::wall|StructureFlags::urban, globals);
	};

	const int max_adjacency = 1;
	auto valid_rural_tile = [&map, &globals, &calc_rural_adjacency, max_adjacency](Vec2i tile_pos) -> bool
	{
		const auto& tile = map.get_tile_or_empty(tile_pos);
		if (tile.structure || (globals.terrain_types.get(tile.terrain).flags & TileTypeFlags::water))
		{
			return false;
		}
		if (calc_rural_adjacency(tile_pos) > max_adjacency)
		{
			return false;
		}
		return true;
	};

	auto valid_rural_tile_bounds = [&map, &globals, &bounds, &urban_occupied](Vec2i cell_rel_pos, Vec2i& out_tile_top_left, Vec2i& out_bounds_size)
	{
		const auto cell_pos = bounds.min + cell_rel_pos;
		out_tile_top_left = cell_to_tile_coords(cell_pos);
		out_bounds_size = { map_cell_size, map_cell_size};
		// TODO: need exclude 1 corner from the diagonal direction as well
		// for (const auto offset : adjacent_offsets)
		// {
		// 	if (urban_occupied(cell_rel_pos + offset))
		// 	{
		// 		if (offset.x != 0)
		// 		{
		// 			out_bounds_size.x--;
		// 			if (offset.x < 0) { out_tile_top_left.x += 1; }
		// 		}
		// 		else
		// 		{
		// 			out_bounds_size.y--;
		// 			if (offset.y < 0) { out_tile_top_left.y += 1; }
		// 		}
		// 	}
		// }
	};

	auto valid_new_rural_cell = [&occupied](const Vec2i& rel_cell_pos)
	{
		if (!occupied(rel_cell_pos))
		{
			for (const auto offset : adjacent_offsets)
			{
				if (occupied(rel_cell_pos + offset))
				{
					return true;
				}
			}
		}
		return false;
	};

	struct Candidate
	{
		Vec2i pos;
		int score;
	};

	// find all existing cells that contain potential rural spaces
	auto available_cells = make_temp_array<Candidate>(0, std::max(num_city_cells - city.num_urban_cells, (bounds_size.x + 2) * (bounds_size.y + 2) - city.occupied_cells.size()));
	int total_spaces = 0;
	int total_occupied = 0;
	for (int i = city.num_urban_cells; i < city.occupied_cells.size(); i++)
	{
		const auto& cell_pos = city.occupied_cells[i];
		const auto rel_pos = city.occupied_cells[i] - bounds.min;
		Vec2i tile_top_left, valid_cell_size;
		valid_rural_tile_bounds(rel_pos, tile_top_left, valid_cell_size);
				
		int num_spaces = 0;
		for (int ti = 0; ti < valid_cell_size.x * valid_cell_size.y; ti++)
		{
			const auto tile_pos = tile_top_left + Vec2i{ti % valid_cell_size.x, ti / valid_cell_size.x};
			if (valid_rural_tile(tile_pos))
			{
				num_spaces++;
			}
			else if (has_all(tile_structure_flag(map, tile_pos, globals), StructureFlags::rural))
			{
				total_occupied++;
			}
		}

		if (num_spaces > 0)
		{
			total_spaces += num_spaces;
			available_cells.push_back({cell_pos, num_spaces});
		}
	}

	// TODO: maybe it should expand more early on (e.g. less rural dev) and less when there are more rural dev????
	// or maybe it could be constant expansion threshold like 50 then until it gets to 5 cells ish, it will keep expand and the more cell you have the less you need expand, i dont know...
	// expand if no available existing cell or occupancy is over limit
	bool use_new_rural_cell = false;
	// const int expansion_threshold = (int)std::floor(total_occupied * 1.f);
	const int expansion_threshold = (total_occupied % 8) * 16 + (int)std::floor(total_occupied * 1.0f);
	// printf("-- threshold now: %d\n", expansion_threshold);
	if (total_spaces <= expansion_threshold)
	{
		auto expansion_bounds = rel_bounds;
		pad_box(expansion_bounds, 1);
		auto expansion_size = box_size(expansion_bounds);
		for (int i = 0; i < expansion_size.x * expansion_size.y; i++)
		{
			const auto cell_rel_pos = expansion_bounds.min + Vec2i{i % expansion_size.x, i / expansion_size.x};
			if (valid_new_rural_cell(cell_rel_pos))
			{
				// TODO: adjacency requirements (or maybe just find any surrounding occupied cell, just don't completely dangle)

				Vec2i tile_top_left, valid_cell_size;
				valid_rural_tile_bounds(cell_rel_pos, tile_top_left, valid_cell_size);
				bool found_space = false;
				for (int ti = 0; ti < valid_cell_size.x * valid_cell_size.y; ti++)
				{
					const auto tile_pos = tile_top_left + Vec2i{ti % valid_cell_size.x, ti / valid_cell_size.x};
					if (valid_rural_tile(tile_pos))
					{
						found_space = true;
						break;
					}
				}

				if (found_space)
				{
					const auto cell_pos = bounds.min + cell_rel_pos;
					const auto dist = manhattan_length(cell_rel_pos - center_cell_rel);
					// TODO: maybe count number of adjacencies as well
					int score = (max_expansion_dist + 1 - dist); // score 1 at max dist, +1 for each step closer
					asserts(score > 0);

					// clear all available existing cells, now use new cells
					if (!use_new_rural_cell)
					{
						use_new_rural_cell = true;
						available_cells.clear();
					}
					available_cells.push_back({cell_pos, score});
				}
			}
		}
	}

	if (!available_cells.empty())
	{
		// once cell is determined, roll from available tiles within the chosen cell
		const auto chosen_idx = random_weighted_array_index(available_cells);
		const auto chosen_cell_pos = available_cells[chosen_idx].pos;

		if (use_new_rural_cell)
		{
			city.occupied_cells.push_back(chosen_cell_pos);
		}

		const auto cell_rel_pos = chosen_cell_pos - bounds.min;
		Vec2i tile_top_left, valid_cell_size;
		valid_rural_tile_bounds(cell_rel_pos, tile_top_left, valid_cell_size);
		auto available_tiles = make_temp_array<Candidate>(0, valid_cell_size.x * valid_cell_size.y);
		for (int ti = 0; ti < valid_cell_size.x * valid_cell_size.y; ti++)
		{
			const auto tile_pos = tile_top_left + Vec2i{ti % valid_cell_size.x, ti / valid_cell_size.x};
			if (valid_rural_tile(tile_pos))
			{
				const auto adjacency = calc_rural_adjacency(tile_pos);
				auto score = std::max((max_adjacency - adjacency) * (max_adjacency - adjacency) * 8, 1);
				available_tiles.push_back({tile_pos, score});
			}
		}

		asserts(!available_tiles.empty());
		return random_weighted_array_element(available_tiles).pos;

	}
	else
	{
		// TODO: consolidate if no more space to fill
		asserts(false);
		return {};
	}


}

static inline void relocate_rural_tile(City& city, Map& map, Vec2i tile_pos, Id rural_struct_type, const Globals& globals)
{
	const auto flags = globals.structure_types.get(rural_struct_type).flags;
	asserts(has_all(flags, StructureFlags::dev|StructureFlags::rural));
	int dev_idx = -1;
	const auto urban_dev = urban_development_level(city);
	const auto total_dev = total_development_level(city);
	for (int i = urban_dev; i < total_dev; i++)
	{
		if (city.dev_tiles[i] == tile_pos)
		{
			dev_idx = i;
			break;
		}
	}
	asserts(dev_idx >= 0);
	const auto new_rural_tile = make_rural_space(city, map, globals);
	asserts(new_rural_tile != tile_pos);
	city.dev_tiles[dev_idx] = new_rural_tile;
	map.set_structure(new_rural_tile, rural_struct_type);
	map.update_glyphs(to_box(new_rural_tile), globals);
}

static inline void relocate_rural_cell(City& city, Map& map, Vec2i cell_pos, const Globals& globals)
{
	bool relocated_any = false;
	const auto tile_top_left = cell_to_tile_coords(cell_pos);
	for (int ti = 0; ti < map_cell_size * map_cell_size; ti++)
	{
		const auto tile_pos = tile_top_left + Vec2i{ti % map_cell_size, ti / map_cell_size};
		const auto structure = map.get_tile_or_empty(tile_pos).structure;
		const auto flags = globals.structure_types.get(structure).flags;
		// TODO: multi-tile structures
		if (has_all(flags, StructureFlags::rural))
		{
			relocate_rural_tile(city, map, tile_pos, structure, globals);
			map.set_structure(tile_pos, null_id);
			relocated_any = true;
		}
	}

	if (relocated_any)
	{
		map.update_glyphs(make_box_wh(tile_top_left.x, tile_top_left.y, map_cell_size, map_cell_size), globals);
	}
}


static inline bool expandable_urban_tile(const Map& map, Vec2i tile_pos, const Globals& globals)
{
	bool adjacent_to_urban_tiles = false;
	for (const auto offset : adjacent_offsets)
	{
		if (is_urban_tile(map, tile_pos + offset, globals))
		{
			adjacent_to_urban_tiles = true;
			break;
		}
	}
	return adjacent_to_urban_tiles && valid_urban_tile(map, tile_pos, globals) && !is_urban_tile(map, tile_pos, globals);
}

static bool should_fill_vacancy(const Map& map, Vec2i tile_pos, const Globals& globals)
{
	if (is_urban_tile(map, tile_pos, globals) || !valid_urban_tile(map, tile_pos, globals))
	{
		return false;
	}

	auto should_fill_with_delta = [&tile_pos, &map, &globals](Vec2i delta) -> bool
	{
		uint8_t mask = 0;
		for (int i = 0; i < 8; i++)
		{
			auto offset = surrounding_offsets[i];
			if (offset.x * delta.x > 0) { offset.x += delta.x; }
			if (offset.y * delta.y > 0) { offset.y += delta.y; }

			const auto& tile = map.get_tile_or_empty(tile_pos + offset);
			if (tile.structure && has_all(globals.structure_types.get(tile.structure).flags, StructureFlags::urban))
			{
				mask |= surrounding_bit_masks[i];
			}
		}
		return urban_mask.should_fill[mask];
	};

	return (
		should_fill_with_delta({0, 0}) ||
		should_fill_with_delta({-1, 0}) ||
		should_fill_with_delta({1, 0}) ||
		should_fill_with_delta({0, -1}) ||
		should_fill_with_delta({-1, -1}) ||
		should_fill_with_delta({1, -1}) ||
		should_fill_with_delta({1, 1}) ||
		should_fill_with_delta({-1, 1}));
}

static void add_vacant_tile(City& city, Map& map, Vec2i tile_pos, const Globals& globals, Array<Vec2i>& append_new_vacant_tiles, Array<Vec2i>& append_additional_vacant_tiles_to_add)
{
	// const auto starting_vacant_tile_idx = append_new_vacant_tiles.size();

	map.set_structure(tile_pos, globals.defines.urban_vacancy_type);
	append_new_vacant_tiles.push_back(tile_pos);
	auto to_update_bounds = to_box(tile_pos);

	// printf("-- added at %d %d\n", tile_pos.x, tile_pos.y);

	for (int i = 0; i < 5 * 5; i++)
	{
		const auto other_tile_pos = tile_pos + Vec2i{i % 5 - 2, i / 5 - 2};
		if (should_fill_vacancy(map, other_tile_pos, globals))
		{
			map.set_structure(other_tile_pos, globals.defines.urban_vacancy_type);
			append_new_vacant_tiles.push_back(other_tile_pos);
			expand_box(to_update_bounds, other_tile_pos);
			// printf("   filled at %d %d\n", other_tile_pos.x, other_tile_pos.y);
		}
	}

	auto check_loop_bounds = to_update_bounds;
	pad_box(check_loop_bounds, 1);

	// fill walls
	for (int i = 0; i < 5 * 5; i++)
	{
		const auto other_tile_pos = tile_pos + Vec2i{i % 5 - 2, i / 5 - 2};
		if (!is_urban_tile(map, other_tile_pos, globals))
		{
			for (const auto offset : surrounding_offsets)
			{
				if (is_urban_tile(map, other_tile_pos + offset, globals))
				{
					const auto orig_struct = map.get_tile_or_empty(other_tile_pos).structure;
					const auto orig_struct_flags = globals.structure_types.get(orig_struct).flags;
					map.set_structure(other_tile_pos, globals.defines.starting_wall_type); // TODO: use city wall type
					expand_box(to_update_bounds, other_tile_pos);
					if (has_all(orig_struct_flags, StructureFlags::rural))
					{
						relocate_rural_tile(city, map, other_tile_pos, orig_struct, globals);
					}
					break;
				}
			}
		}
	}
	map.update_glyphs(to_update_bounds, globals);

	// check surrounding tiles to make sure no loops being created
	const auto size = box_size(check_loop_bounds);
	for (int i = 0; i < size.x * size.y; i++)
	{
		const auto other_tile_pos = check_loop_bounds.min + Vec2i{i % size.x, i / size.x};
		if (!is_urban_tile(map, other_tile_pos, globals))
		{
			const auto mask = calc_surrounding_structure_mask(map, other_tile_pos, StructureFlags::urban, globals);
			// printf("   * checked at %d %d, mask %xu\n", other_tile_pos.x, other_tile_pos.y, mask);
			if (mask == (0x02 | 0x20) ||
				mask == (0x08 | 0x80) ||
				mask == (0x01 | 0x08) ||
				mask == (0x01 | 0x20) ||
				mask == (0x04 | 0x20) ||
				mask == (0x04 | 0x80) ||
				mask == (0x10 | 0x80) ||
				mask == (0x10 | 0x02) ||
				mask == (0x40 | 0x02) ||
				mask == (0x40 | 0x08))
			{
				// printf("!!!!!!!!!!!!!!!!!!!!!!! additiona added!!!!!!!!!!!!!!!!!!!\n");
				append_additional_vacant_tiles_to_add.push_back(other_tile_pos);
			}
		}
	}
}

static inline bool is_urban_cell(const City& city, Vec2i cell_pos)
{
	for (int i = 0; i < city.num_urban_cells; i++)
	{
		if (city.occupied_cells[i] == cell_pos)
		{
			return true;
		}
	}
	return false;
}

static inline void add_urban_cell(City& city, Map& map, Vec2i cell_pos, const Globals& globals)
{
	// relocate if the cell is occupied by rural area
	bool replacing_rural_cell = false;
	for (int i = city.num_urban_cells; i < city.occupied_cells.size(); i++)
	{
		if (city.occupied_cells[i] == cell_pos)
		{
			std::swap(city.occupied_cells[city.num_urban_cells], city.occupied_cells[i]);
			city.num_urban_cells++;
			relocate_rural_cell(city, map, cell_pos, globals);
			return;
		}
	}

	// not replacing rural cells
	city.occupied_cells.insert(city.num_urban_cells, cell_pos);
	city.num_urban_cells++;
}

static bool expand_urban_vacant_tile_in_cell(City& city, Map& map, Vec2i cell_pos, const Globals& globals, Array<Vec2i>& append_new_vacant_tiles)
{
	auto expand_tiles = make_temp_array<Vec2i>(0, map_cell_size);
	const auto tile_top_left = cell_to_tile_coords(cell_pos);
	for (int ti = 0; ti < map_cell_size * map_cell_size; ti++)
	{
		const auto tile_pos = tile_top_left + Vec2i{ti % map_cell_size, ti / map_cell_size};
		if (expandable_urban_tile(map, tile_pos, globals))
		{
			expand_tiles.push_back(tile_pos);
		}
	}

	if (expand_tiles.empty())
	{
		return false;
	}

	const auto new_tile = expand_tiles[rand_int(expand_tiles.size())];
	
	auto additional_vacant_tiles = make_temp_array<Vec2i>(0, 4);
	add_vacant_tile(city, map, new_tile, globals, append_new_vacant_tiles, additional_vacant_tiles);

	for (int i = 0; i < additional_vacant_tiles.size(); i++)
	{
		const auto additional_tile_pos = additional_vacant_tiles[i];
		if (!is_urban_tile(map, additional_tile_pos, globals))
		{
			const auto additional_cell_pos = tile_to_cell_coords(additional_tile_pos);
			if (!is_urban_cell(city, additional_cell_pos))
			{
				add_urban_cell(city, map, additional_cell_pos, globals);
			}
			add_vacant_tile(city, map, additional_tile_pos, globals, append_new_vacant_tiles, additional_vacant_tiles);		
		}
	}

#if 0
	map.set_structure(new_tile, globals.defines.urban_vacancy_type);
	append_new_vacant_tiles.push_back(new_tile);
	auto to_update_bounds = to_box(new_tile);

	for (int i = 0; i < 5 * 5; i++)
	{
		const auto other_tile_pos = new_tile + Vec2i{i % 5 - 2, i / 5 - 2};
		if (should_fill_vacancy(map, other_tile_pos, globals))
		{
			map.set_structure(other_tile_pos, globals.defines.urban_vacancy_type);
			append_new_vacant_tiles.push_back(other_tile_pos);
			expand_box(to_update_bounds, other_tile_pos);
		}
	}

	for (int i = 0; i < 5 * 5; i++)
	{
		const auto other_tile_pos = new_tile + Vec2i{i % 5 - 2, i / 5 - 2};
		if (!is_urban_tile(map, other_tile_pos, globals))
		{
			for (const auto offset : surrounding_offsets)
			{
				if (is_urban_tile(map, other_tile_pos + offset, globals))
				{
					map.set_structure(other_tile_pos, globals.defines.starting_wall_type); // TODO: use city wall type
					expand_box(to_update_bounds, other_tile_pos);
					break;
				}
			}
		}
	}

	// for (const auto offset : surrounding_offsets)
	// {
	// 	const auto other_tile_pos = new_tile + offset;
	// 	if (!is_urban_tile(map, other_tile_pos, globals))
	// 	{
	// 		// const auto& other_tile = map.get_tile_or_empty(other_tile_pos);
	// 		// const auto flags = globals.structure_types.get(other_tile.structure).flags;

	// 		// check if should fill vacancy
	// 		const auto mask = calc_surrounding_structure_mask(map, other_tile_pos, StructureFlags::urban, globals);
	// 		if (urban_mask.should_fill[mask])
	// 		{
	// 			map.set_structure(other_tile_pos, globals.defines.urban_vacancy_type);
	// 			append_new_vacant_tiles.push_back(other_tile_pos);
	// 		}
	// 		else
	// 		{
	// 			// otherwise is wall position (since it's surrounding an urban tile)
	// 			map.set_structure(other_tile_pos, globals.defines.starting_wall_type); // TODO: use city's wall type				
	// 		}
	// 	}
	// }
	map.update_glyphs(to_update_bounds, globals);

#endif
	return true;
}

static Vec2i make_urban_space(City& city, Map& map, const Globals& globals)
{
	const auto bounds = urban_cells_bounds(city);
	const auto bounds_size = box_size(bounds);
	const auto rel_bounds = make_box_wh(0, 0, bounds_size.x, bounds_size.y);
	auto urban_cells = make_temp_array<uint8_t>(bounds_size.x * bounds_size.y);
	const auto center_cell = tile_to_cell_coords(city.center);
	const auto center_cell_rel = center_cell - bounds.min;

	struct ExpansionCell
	{
		Vec2i rel_pos;
		int score;
	};
	auto available_cells = make_temp_array<ExpansionCell>(0, (bounds_size.x + 2) * (bounds_size.y + 2) - city.num_urban_cells);
	auto vacant_tiles = make_temp_array<Vec2i>(0, city.num_urban_cells * map_cell_size * map_cell_size);
	int num_occupied = 0;
	int max_expansion_dist = 0;
	for (int i = 0; i < city.num_urban_cells; i++)
	{
		const auto& cell_pos = city.occupied_cells[i];
		const auto rel_pos = city.occupied_cells[i] - bounds.min;
		const int idx = (rel_pos.y * bounds_size.x + rel_pos.x);
		urban_cells[idx] = 1;
		max_expansion_dist = std::max(max_expansion_dist, manhattan_length(rel_pos - center_cell_rel));

		// TODO: make an iterator function
		bool found_space_for_expansion = false;
		const auto tile_top_left = cell_to_tile_coords(cell_pos);
		for (int ti = 0; ti < map_cell_size * map_cell_size; ti++)
		{
			const auto tile_pos = tile_top_left + Vec2i{ti % map_cell_size, ti / map_cell_size};
			if (is_urban_tile(map, tile_pos, globals))
			{
				const auto& tile = map.get_tile_or_empty(tile_pos);
				const auto struct_type = tile.structure;
				const auto flags = globals.structure_types.get(struct_type).flags;
				if (has_all(flags, StructureFlags::vacancy))
				{
					vacant_tiles.push_back(tile_pos);
				}
				else
				{
					num_occupied++;
				}
			}
			else if (expandable_urban_tile(map, tile_pos, globals))
			{
				found_space_for_expansion = true;
			}
		}
		if (found_space_for_expansion)
		{
			available_cells.push_back({ rel_pos, 0 }); // score calculated later
		}
	}
	max_expansion_dist += 1; // max distance from potential expansion cell to the center
	const auto num_existing_available_cells = available_cells.size();

	auto occupied = [&rel_bounds, &bounds_size, &urban_cells](const Vec2i& pos)
	{
		return encompasses(rel_bounds, pos) && (urban_cells[pos.y * bounds_size.x + pos.x] == 1);
	};

	const int expansion_threshold = (int)std::floor(num_occupied * 0.5f);
	if (vacant_tiles.size() <= expansion_threshold)
	{
		// expand
		// TODO: make an iterator function
		for (int y = -1; y < bounds_size.y + 1; y++)
		{
			for (int x = -1; x < bounds_size.x + 1; x++)
			{
				// TODO: we cannot expand into other city's territory (either claimed or occupied)
				const Vec2i rel_pos{x, y};
				if (!occupied(rel_pos))
				{
					bool adjacent = false;
					for (const auto& offset : adjacent_offsets)
					{
						const auto other_pos = rel_pos + offset;
						if (occupied(other_pos))
						{
							adjacent = true;
							break;
						}
					}

					if (adjacent)
					{
						const auto cell_pos = (bounds.min + rel_pos);
						const auto tile_top_left = cell_to_tile_coords(cell_pos);
						for (int ti = 0; ti < map_cell_size * map_cell_size; ti++)
						{
							const auto tile_pos = tile_top_left + Vec2i{ti % map_cell_size, ti / map_cell_size};
							if (expandable_urban_tile(map, tile_pos, globals))
							{
								available_cells.push_back({rel_pos, 0}); // score calculated later
								break;
							}

						}
					}
				}
			}
		}

		if (!available_cells.empty())
		{
			int total_score = 0;
			for (auto& expand_cell : available_cells)
			{
				const auto dist = manhattan_length(expand_cell.rel_pos - center_cell_rel);
				// TODO: maybe count number of adjacencies as well
				expand_cell.score = (max_expansion_dist + 1 - dist); // score 1 at max dist, +1 for each step closer
				int num_sides_connected = 0;
				for (const auto offset : adjacent_offsets)
				{
					if (occupied(expand_cell.rel_pos + offset))
					{
						num_sides_connected++;
					}
				}
				expand_cell.score += std::max(1, num_sides_connected) - 1;
				total_score += expand_cell.score;
			}		

			const auto roll = rand_int(total_score);
			Vec2i new_cell_pos;
			int running = 0;
			size_t chosen_cell_idx = 0;
			for (const auto& cell : available_cells)
			{
				running += cell.score;
				if (roll < running)
				{
					new_cell_pos = (bounds.min + cell.rel_pos);
					break;
				}
				chosen_cell_idx++;
			}

			// TODO: relocate if this is a rural cell

			// mark urban cell if chose a new cell
			if (chosen_cell_idx >= num_existing_available_cells)
			{
				add_urban_cell(city, map, new_cell_pos, globals);
			}

			// expand a number of times if until no more available space
			const int min_expansions = 2;
			const int max_expansions = 4;
			const auto num_expansions = rand_int(min_expansions, max_expansions);
			for (int i = 0; i < num_expansions; i++)
			{
				if (!expand_urban_vacant_tile_in_cell(city, map, new_cell_pos, globals, vacant_tiles))
				{
					break;
				}
			}

			// NOTE: we can't do this, since the chosen cell may not be fully new and may contain existing vacant tiles
			// collect all the newly added urban vacant tiles in the new cell
			// const auto tile_top_left = cell_to_tile_coords(new_cell_pos);
			// for (int ti = 0; ti < map_cell_size * map_cell_size; ti++)
			// {
			// 	const auto tile_pos = tile_top_left + Vec2i{ti % map_cell_size, ti / map_cell_size};
			// 	const auto& tile = map.get_tile_or_empty(tile_pos);
			// 	const auto flags = globals.structure_types.get(tile.structure).flags;
			// 	if (has_all(flags, (StructureFlags::urban | StructureFlags::vacancy)))
			// 	{
			// 		vacant_tiles.push_back(tile_pos);
			// 	}
			// }
		}
	}

	if (!vacant_tiles.empty())
	{
		// pick a random vacant tile
		const auto roll = rand_int(vacant_tiles.size());
		return vacant_tiles[roll];
	}
	else
	{
		// TODO: consolidate when the whole territory is full
		// TODO: also we need leave spaces for rural territory
		asserts(false);
		return {};
	}
}

void develop_city(World& world, Id city_id, DevelopmentArea area, const Globals& globals)
{
	auto& city = world.cities.get(city_id);
	auto& map = world.map;
	Vec2i dev_pos;
	if (area == DevelopmentArea::urban)
	{
		dev_pos = make_urban_space(city, map, globals);
	}
	else
	{
		dev_pos = make_rural_space(city, map, globals);
	}
	add_development(city, map, area, dev_pos, globals);
	world.map.update_glyphs(to_box(dev_pos), globals);
	city.urban_bounds = urban_tile_bounds(city, world.map, globals);
}

namespace serialization
{
	struct MapTilesBlob;

	struct UrbanStructureTiles
	{
		template<class TOp>
		static inline void write(TOp& op, const City& city, Id type)
		{
			const Globals& globals = *op.context.globals;
			const World& world = *op.context.world;
			auto tiles = make_temp_array<Vec2i>(0, city.occupied_cells.size() * map_cell_size);
			for (int i = 0; i < city.num_urban_cells; i++)
			{
				const auto tile_top_left = cell_to_tile_coords(city.occupied_cells[i]);
				for (int ti = 0; ti < map_cell_size * map_cell_size; ti++)
				{
					const auto tile_pos = tile_top_left + Vec2i{ti % map_cell_size, ti / map_cell_size};
					const auto& tile = world.map.get_tile_or_empty(tile_pos);
					if (tile.structure == type)
					{
						tiles.push_back(tile_pos);
					}
				}
			}
			op.value(tiles);
		}

		template<class TOp>
		static inline void read(TOp& op, City& city, Id type)
		{
			const Globals& globals = *op.context.globals;
			World& world = *op.context.world;
			auto tiles = make_temp_array<Vec2i>(0, city.occupied_cells.size() * map_cell_size);
			op.value(tiles);
			auto bounds = to_box(city.center);
			for (const auto tile_pos : tiles)
			{
				world.map.set_structure(tile_pos, type);
				expand_box(bounds, tile_pos);
			}

			world.map.update_glyphs(bounds, globals);
		}
	};

	template<class T>
	static void serialize(T& op, RefType<T, City> city)
	{
		op.prop("name", city.name);
		op.prop("center", city.center);
		op.prop("dev_urban", city.development_level[(int)DevelopmentArea::urban]);
		op.prop("dev_rural", city.development_level[(int)DevelopmentArea::rural]);
		op.prop("num_urban_cells", city.num_urban_cells);
		op.prop("cells", city.occupied_cells);
		op.prop("dev_tiles", city.dev_tiles);
		op.prop("vacant_tiles", city, UrbanStructureTiles{}, op.context.globals->defines.urban_vacancy_type);
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
		struct
		{
			const Globals* globals{};
			std::remove_reference_t<decltype(world)>* world{};
		}
		context{&globals, &world};

		auto op = make_file_op<E>(format_str("%s/%s", path, "map.dat"), context);
		serialize(op, world.map);

		op = make_file_op<E>(format_str("%s/%s", path, "world.dat"), context);
		op.section("cities");
		op.collection(world.cities, "city");
	}
}



void save_world(const World& world, const String& path, const Globals& globals)
{
	serialization::serialize_world<SerializeOpType::write>(world, path, globals);
}

World load_world(const String& path, const Globals& globals, Allocator alloc)
{
	scoped_context_allocator(alloc);
	World world;
	world.init(alloc);
	serialization::serialize_world<SerializeOpType::read>(world, path, globals);
	for (auto& city : world.cities)
	{
		const auto dev_level = total_development_level(city);
		int start = 0;
		for (int area = 0; area < (int)DevelopmentArea::count; area++)
		{
			const auto dev_level = city.development_level[area];
			const auto& dev_type = globals.development_types[area];
			for (int i = start; i < start + dev_level; i++)
			{
				const auto dev_pos = city.dev_tiles[i];
				world.map.set_structure(dev_pos, dev_type.structure_type);
				world.map.update_glyphs(to_box(dev_pos), globals);
			}
			start += dev_level;
		}
		setup_walls(city, world.map, globals);
		city.urban_bounds = urban_tile_bounds(city, world.map, globals);
	}
	return world;
}


