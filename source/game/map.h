#pragma once

#include <cstdint>
#include "engine/array.h"
#include "engine/string.h"
#include "engine/color.h"
#include "engine/math_types.h"

struct TileType
{
	String name;
	uint16_t glyph;
	Color color_a;
	Color color_b;
};

struct Tile
{
	uint16_t type{};
};

struct MapChunk
{
	Id first_tile_id{};
};

static constexpr const int map_chunk_size = 32;

struct Map
{
	IRect chunk_bounds;
	SimpleArray<MapChunk> chunks;
	SimpleArray<Tile> tiles;

	inline Id tile_id(const IVec2& coords) const;
	inline int chunk_coords_to_idx(const IVec2& coords) const;
	inline IVec2 chunk_idx_to_coords(int idx) const;
	inline void set_tile(const IVec2& coords, const Tile& tile);
	void expand_to_fit_chunk(const IVec2 coords);
};

inline int tile_to_chunk(int tile_coord)
{
	return tile_coord >= 0 ? (tile_coord / map_chunk_size) : 
		-((-tile_coord + map_chunk_size - 1) / map_chunk_size);
}

inline int chunk_first_tile(int chunk_coord)
{
	return chunk_coord * map_chunk_size;
}

inline Id Map::tile_id(const IVec2& coords) const
{
	IVec2 chunk_coords{ tile_to_chunk(coords.x), tile_to_chunk(coords.y) };
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

inline int Map::chunk_coords_to_idx(const IVec2& coords) const
{
	return (coords.x - chunk_bounds.x) + (coords.y - chunk_bounds.y) * chunk_bounds.width;
}

inline IVec2 Map::chunk_idx_to_coords(int idx) const
{
	return 
	{
		chunk_bounds.x + (idx % chunk_bounds.width),
		chunk_bounds.y + (idx / chunk_bounds.width),
	};
}

inline void Map::set_tile(const IVec2& coords, const Tile& tile)
{
	IVec2 chunk_coords{ tile_to_chunk(coords.x), tile_to_chunk(coords.y) };
	if (chunk_coords.x < chunk_bounds.x || chunk_coords.x >= (chunk_bounds.x + chunk_bounds.width) ||
		chunk_coords.y < chunk_bounds.y || chunk_coords.y >= (chunk_bounds.y + chunk_bounds.height))
	{
		// pad chunks to fit the required chunk
		expand_to_fit_chunk(chunk_coords);
	}

	int chunk_idx = chunk_coords_to_idx(chunk_coords);
	Id& first_tile_id = chunks[chunk_idx].first_tile_id;
	if (!first_tile_id)
	{
		// add tiles for the chunk
		first_tile_id = index_to_id(tiles.size());
		tiles.insert_zeroes(tiles.size(), map_chunk_size * map_chunk_size);
	}

	Id tile_id = first_tile_id + (coords.x - chunk_first_tile(chunk_coords.x)) +
		(coords.y - chunk_first_tile(chunk_coords.y)) * map_chunk_size;
	tiles[id_to_index(tile_id)] = tile;
}


