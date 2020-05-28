#pragma once

#include "globals.h"

enum TileTypeFlags: uint32_t
{
	water = 0x0001,
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

static constexpr const int map_chunk_size = 32;

struct Map
{
	IRect chunk_bounds;
	Array<MapChunk> chunks;
	Array<Tile> tiles;

	Array<TileGlyph> ground_glyphs;

	inline Id tile_id(const Vec2i& coords) const;
	inline int chunk_coords_to_idx(const Vec2i& coords) const;
	inline Vec2i chunk_idx_to_coords(int idx) const;
	inline void set_tile(const Vec2i& coords, const Tile& tile);
	inline void set_terrain(const Vec2i& coords, TypeIndex terrain_type);
	inline void set_structure(const Vec2i& coords, TypeIndex structure_type);

	void expand_to_fit_chunk(const Vec2i& coords);

	// NOTE: dirty_tiles should just be the tiles that changed, this function will 
	// auto expand to surounding tiles that also need glyph updates
	void update_glyphs(const Box2i dirty_tiles, const Globals& globals);
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

inline Vec2i Map::chunk_idx_to_coords(int idx) const
{
	return 
	{
		chunk_bounds.x + (idx % chunk_bounds.width),
		chunk_bounds.y + (idx / chunk_bounds.width),
	};
}

inline void Map::set_tile(const Vec2i& coords, const Tile& tile)
{
	Vec2i chunk_coords{ tile_to_chunk(coords.x), tile_to_chunk(coords.y) };
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
		asserts(tiles.size() == ground_glyphs.size());
		tiles.resize(tiles.size() + (map_chunk_size * map_chunk_size));
		ground_glyphs.resize(tiles.size());
	}

	Id tile_id = first_tile_id + (coords.x - chunk_first_tile(chunk_coords.x)) +
		(coords.y - chunk_first_tile(chunk_coords.y)) * map_chunk_size;
	tiles[id_to_index(tile_id)] = tile;
}

inline void Map::set_terrain(const Vec2i& coords, TypeIndex terrain_type)
{
	const auto id = tile_id(coords);
	if (id)
	{
		tiles[id_to_index(id)].terrain = terrain_type;
	}
	else
	{
		Tile new_tile{};
		new_tile.terrain = terrain_type;
		set_tile(coords, new_tile);
	}
}

inline void Map::set_structure(const Vec2i& coords, TypeIndex structure_type)
{
	const auto id = tile_id(coords);
	if (id)
	{
		tiles[id_to_index(id)].structure = structure_type;
	}
	else
	{
		Tile new_tile{};
		new_tile.structure = structure_type;
		set_tile(coords, new_tile);
	}
}

