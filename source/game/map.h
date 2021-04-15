#pragma once

#include "globals.h"
#include "engine/enum_utils.h"

enum TileTypeFlags: uint32_t
{
	water = 0x00000001,
};

struct Tile
{
	uint32_t terrain{};
	uint32_t structure{};
};

struct MapChunk
{
	Id first_tile_id{};
};

struct TileGlyph
{
	uint16_t code;
	Color32 color;
};

static inline constexpr const int map_chunk_size = 64;
static inline constexpr const int map_chunk_tile_count = (map_chunk_size * map_chunk_size);
static inline constexpr const int map_cell_size = 4;
static inline constexpr const int foo = -1 % 2;

// surrounding means for all 8 directions (including diagonal)
//     7 0 1    | -> +X
//     6 - 2    V 
//     5 4 3    +Y 
static inline constexpr const Vec2i surrounding_offsets[] = { {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1} };
static inline constexpr const uint8_t surrounding_bit_masks[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
// adjacency means for all 4 directions (non-diagonal)
//       0      | -> +X
//     3 - 1    V 
//       2      +Y 
static inline constexpr const Vec2i adjacent_offsets[] = { {0, -1}, {1, 0}, {0, 1}, {-1, 0} };
static inline constexpr const uint8_t adjacent_bit_masks[] = { 0x01, 0x02, 0x04, 0x08 };

struct Map
{
	IRect chunk_bounds;
	Array<MapChunk> chunks;
	Array<Tile> tiles;

	Array<TileGlyph> ground_glyphs;

	static inline const Tile empty_tile{};

	inline void alloc(Allocator allocator)
	{
		chunks.alloc(0, 1024, allocator);
		tiles.alloc(0, 512 * 512, allocator);
		ground_glyphs.alloc(0, 512 * 512, allocator);
	}

	inline Id tile_id(const Vec2i& coords) const;
	inline int chunk_coords_to_idx(const Vec2i& coords) const;
	inline Vec2i chunk_idx_to_coords(size_t idx) const;
	inline const Tile* get_tile(const Vec2i& coords) const { return const_cast<Map*>(this)->get_tile(coords); } 
	inline Tile* get_tile(const Vec2i& coords);
	inline const Tile& get_tile_or_empty(const Vec2i& coords) const;
	inline void set_tile(const Vec2i& coords, const Tile& tile);
	inline void set_terrain(const Vec2i& coords, Id terrain_type);
	inline void set_structure(const Vec2i& coords, Id structure_type);

	inline Id ensure_tiles_for_chunk(const Vec2i& coords);
	void expand_to_fit_chunk(const Vec2i& coords);

	// NOTE: dirty_tiles should just be the tiles that changed, this function will 
	// auto expand to surounding tiles that also need glyph updates
	void update_glyphs(const Box2i dirty_tiles, const Globals& globals);
};

struct MapRef
{
	Map& map;
	const Globals& globals;
};

static inline uint8_t calc_surrounding_structure_mask(const Map& map, Vec2i tile_pos, StructureFlags flags, const Globals& globals);
static inline uint8_t calc_adjacent_structure_mask(const Map& map, Vec2i tile_pos, StructureFlags flags, const Globals& globals);
static inline int count_adjacent_structures_with_any_flag(const Map& map, Vec2i tile_pos, StructureFlags flags, const Globals& globals);
static inline StructureFlags tile_structure_flag(const Map& map, Vec2i tile_pos, const Globals& globals);

inline int tile_to_chunk(int tile_coord)
{
	return tile_coord >= 0 ? (tile_coord / map_chunk_size) : 
		-((-tile_coord + map_chunk_size - 1) / map_chunk_size);
}

inline Vec2i tile_to_chunk_coords(const Vec2i& tile_coords)
{
	return { tile_to_chunk(tile_coords.x), tile_to_chunk(tile_coords.y) };
}

inline int tile_to_cell(int tile_coord)
{
	// TODO: can we use std::div to make this bit cleaner?
	return tile_coord >= 0 ? (tile_coord / map_cell_size) : 
		((tile_coord - map_cell_size + 1) / map_cell_size);
}

inline Vec2i tile_to_cell_coords(const Vec2i& tile_coords)
{
	return { tile_to_cell(tile_coords.x), tile_to_cell(tile_coords.y) };
}

inline Vec2i cell_to_tile_coords(const Vec2i& cell_coords)
{
	return cell_coords * map_cell_size;
}

inline int chunk_first_tile(int chunk_coord)
{
	return chunk_coord * map_chunk_size;
}

inline Id Map::tile_id(const Vec2i& coords) const
{
	Vec2i chunk_coords{ tile_to_chunk(coords.x), tile_to_chunk(coords.y) };
	if (chunk_coords.x < chunk_bounds.x || chunk_coords.x >= (chunk_bounds.x + chunk_bounds.width) ||
		chunk_coords.y < chunk_bounds.y || chunk_coords.y >= (chunk_bounds.y + chunk_bounds.height))
	{
		return null_id;
	}

	Id first_tile_id = chunks[chunk_coords_to_idx(chunk_coords)].first_tile_id;
	if (!first_tile_id) { return null_id; }
	return first_tile_id + (coords.x - chunk_first_tile(chunk_coords.x)) +
		(coords.y - chunk_first_tile(chunk_coords.y)) * map_chunk_size;
}

inline int Map::chunk_coords_to_idx(const Vec2i& coords) const
{
	return (coords.x - chunk_bounds.x) + (coords.y - chunk_bounds.y) * chunk_bounds.width;
}

inline Vec2i Map::chunk_idx_to_coords(size_t idx) const
{
	return 
	{
		(int)(chunk_bounds.x + (idx % chunk_bounds.width)),
		(int)(chunk_bounds.y + (idx / chunk_bounds.width)),
	};
}

inline Tile* Map::get_tile(const Vec2i& coords)
{
	const auto id = tile_id(coords);
	if (id)
	{
		return &tiles[id_to_index(id)];
	}
	return nullptr;
}

inline const Tile& Map::get_tile_or_empty(const Vec2i& coords) const
{
	auto tile = get_tile(coords);
	return tile ? *tile : Map::empty_tile;
}

inline Id Map::ensure_tiles_for_chunk(const Vec2i& coords)
{
	if (coords.x < chunk_bounds.x || coords.x >= (chunk_bounds.x + chunk_bounds.width) ||
		coords.y < chunk_bounds.y || coords.y >= (chunk_bounds.y + chunk_bounds.height))
	{
		// pad chunks to fit the required chunk
		expand_to_fit_chunk(coords);
	}

	int chunk_idx = chunk_coords_to_idx(coords);
	Id& first_tile_id = chunks[chunk_idx].first_tile_id;
	if (!first_tile_id)
	{
		// add tiles for the chunk
		first_tile_id = index_to_id(tiles.size());
		tiles.resize(tiles.size() + (map_chunk_size * map_chunk_size));
	}
	return first_tile_id;
}

inline void Map::set_tile(const Vec2i& coords, const Tile& tile)
{
	Vec2i chunk_coords{ tile_to_chunk(coords.x), tile_to_chunk(coords.y) };
	// if (chunk_coords.x < chunk_bounds.x || chunk_coords.x >= (chunk_bounds.x + chunk_bounds.width) ||
	// 	chunk_coords.y < chunk_bounds.y || chunk_coords.y >= (chunk_bounds.y + chunk_bounds.height))
	// {
	// 	// pad chunks to fit the required chunk
	// 	expand_to_fit_chunk(chunk_coords);
	// }

	// int chunk_idx = chunk_coords_to_idx(chunk_coords);
	// Id& first_tile_id = chunks[chunk_idx].first_tile_id;
	// if (!first_tile_id)
	// {
	// 	// add tiles for the chunk
	// 	first_tile_id = index_to_id(tiles.size());
	// 	asserts(tiles.size() == ground_glyphs.size());
	// 	tiles.resize(tiles.size() + (map_chunk_size * map_chunk_size));
	// 	ground_glyphs.resize(tiles.size());
	// }

	Id first_tile_id = ensure_tiles_for_chunk(chunk_coords);
	Id tile_id = first_tile_id + (coords.x - chunk_first_tile(chunk_coords.x)) +
		(coords.y - chunk_first_tile(chunk_coords.y)) * map_chunk_size;
	tiles[id_to_index(tile_id)] = tile;
}

inline void Map::set_terrain(const Vec2i& coords, Id terrain_type)
{
	const auto id = tile_id(coords);
	if (id)
	{
		// TODO: other 16bit contextual id
		tiles[id_to_index(id)].terrain = to_type_id(terrain_type);
	}
	else
	{
		Tile new_tile{};
		new_tile.terrain = to_type_id(terrain_type);
		set_tile(coords, new_tile);
	}
}

inline void Map::set_structure(const Vec2i& coords, Id structure_type)
{
	const auto id = tile_id(coords);
	if (id)
	{
		// TODO: other 16bit contextual id
		tiles[id_to_index(id)].structure = to_type_id(structure_type);
	}
	else
	{
		Tile new_tile{};
		new_tile.structure = to_type_id(structure_type);
		set_tile(coords, new_tile);
	}
}

static inline uint8_t calc_surrounding_structure_mask(const Map& map, Vec2i tile_pos, StructureFlags flags, const Globals& globals)
{
	uint8_t mask = 0;
	for (int i = 0; i < 8; i++)
	{
		const auto& tile = map.get_tile_or_empty(tile_pos + surrounding_offsets[i]);
		if (tile.structure && has_all(globals.structure_types.get(tile.structure).flags, flags))
		{
			mask |= surrounding_bit_masks[i];
		}
	}
	return mask;
}

static inline uint8_t calc_adjacent_structure_mask(const Map& map, Vec2i tile_pos, StructureFlags flags, const Globals& globals)
{
	uint8_t mask = 0;
	for (int i = 0; i < 4; i++)
	{
		const auto& tile = map.get_tile_or_empty(tile_pos + adjacent_offsets[i]);
		if (tile.structure && has_all(globals.structure_types.get(tile.structure).flags, flags))
		{
			mask |= adjacent_bit_masks[i];
		}
	}
	return mask;
}

static inline int count_adjacent_structures_with_any_flag(const Map& map, Vec2i tile_pos, StructureFlags flags, const Globals& globals)
{
	int num = 0;
	for (int i = 0; i < 4; i++)
	{
		const auto& tile = map.get_tile_or_empty(tile_pos + adjacent_offsets[i]);
		if (tile.structure && has_any(globals.structure_types.get(tile.structure).flags, flags))
		{
			num++;
		}
	}
	return num;
}

static inline StructureFlags tile_structure_flag(const Map& map, Vec2i tile_pos, const Globals& globals)
{
	const auto& tile = map.get_tile_or_empty(tile_pos);
	return tile.structure ? globals.structure_types.get(tile.structure).flags : StructureFlags::none;
}


