#include "map.h"


// mask encodes 1 for the directions that are not water
// total 8 bits for 8 directions, ordered as follows:
//     7 0 1    +Y
//     6 - 2     ^
//     5 4 3     | -> +X
//
// code encodes 1 for neighbors that need water connections
// total 4 bits for 4 neighbors, ordered as follows:
//       0      +Y
//     3 - 1     ^
//       2       | -> +X
struct WaterTileMask
{
	uint8_t codes[256];
	constexpr WaterTileMask(): codes{}
	{
		for (unsigned int i = 0; i < 256; i++)
		{
			uint8_t code = 0;
			uint8_t mask = (uint8_t)i;
			if (mask & 0x01) { code |= (0x02 | 0x08); }
			if (mask & 0x02) { code |= (0x01 | 0x02); }
			if (mask & 0x04) { code |= (0x01 | 0x04); }
			if (mask & 0x08) { code |= (0x02 | 0x04); }
			if (mask & 0x10) { code |= (0x02 | 0x08); }
			if (mask & 0x20) { code |= (0x04 | 0x08); }
			if (mask & 0x40) { code |= (0x01 | 0x04); }
			if (mask & 0x80) { code |= (0x01 | 0x08); }
			if (mask & 0x01) { code &= ~(0x01); }
			if (mask & 0x04) { code &= ~(0x02); }
			if (mask & 0x10) { code &= ~(0x04); }
			if (mask & 0x40) { code &= ~(0x08); }
			codes[i] = code;
		}
	}
};
static const constexpr WaterTileMask water_tile_mask{};


void Map::expand_to_fit_chunk(const Vec2i& coords)
{
	if (chunks.empty())
	{
		chunks.insert_defaults(0, 1);
		chunk_bounds = { coords.x, coords.y, 1, 1 };
	}
	else if (coords.x < chunk_bounds.x || coords.x >= (chunk_bounds.x + chunk_bounds.width))
	{
		int min_x = std::min(coords.x, chunk_bounds.x);
		int min_y = std::min(coords.y, chunk_bounds.y);
		int max_x = std::max(coords.x, chunk_bounds.x + chunk_bounds.width - 1);
		int max_y = std::max(coords.y, chunk_bounds.y + chunk_bounds.height - 1);
		IRect new_bounds{ min_x, min_y, (max_x - min_x + 1), (max_y - min_y + 1) };

		ArrayTemp<MapChunk> temp_chunks{chunks.size(), chunks.size()};
		memcpy(temp_chunks.data(), chunks.data(), chunks.size() * sizeof(MapChunk));

		chunks.clear();
		chunks.resize(new_bounds.width * new_bounds.height);

		for (int y = 0; y < chunk_bounds.height; y++)
		{
			void* src = &temp_chunks[y * chunk_bounds.width];
			void* dst = &chunks[chunk_bounds.x - min_x + (y + chunk_bounds.y - min_y) * new_bounds.width];
			memcpy(dst, src, chunk_bounds.width * sizeof(MapChunk));
		}
		chunk_bounds = new_bounds;
	}
	else if (coords.y < chunk_bounds.y)
	{
		chunks.insert_defaults(0, (chunk_bounds.y - coords.y) * chunk_bounds.width);
		chunk_bounds.height += (chunk_bounds.y - coords.y);
		chunk_bounds.y = coords.y;
	}
	else if (coords.y >= (chunk_bounds.y + chunk_bounds.height))
	{
		chunks.insert_defaults(chunks.size(), (coords.y - chunk_bounds.y - chunk_bounds.height + 1) * chunk_bounds.width);
		chunk_bounds.height = (coords.y - chunk_bounds.y + 1);
	}
	asserts(chunks.size() == (chunk_bounds.width * chunk_bounds.height));
}

// This generates the mask that encodes the non-water directions (see above explanations of mask)
// return 0 for water tile that has no blocking non-water directions, in which case a normal tile glyph should be used
static inline uint8_t calc_neighbor_water_mask(const Map& map, const Array<TerrainType>& terrain_types, const Vec2i& tile_coords)
{
	// remind of ordering:
	//     7 0 1  +Y
	//     6 - 2   ^
	//     5 4 3   | -> +X
	const Vec2i offsets[] = { {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1} };
	uint8_t mask = 0;
	for (int i = 0; i < 8; i++)
	{
		const Id id = map.tile_id(tile_coords + offsets[i]);
		const uint16_t type = (id ? map.tiles[id_to_index(id)].terrain : 0);
		if (!(terrain_types[type].flags & TileTypeFlags::water))
		{
			mask |= (1 << i);
		}
	}
	return mask;
}

static inline uint8_t calc_neighbor_matching_structure_mask(const Map& map, uint32_t tile_struct, const Vec2i& tile_coords)
{
	// remind of ordering:
	//       0    +Y
	//     3 - 1   ^
	//       2     | -> +X
	const Vec2i offsets[] = { {0, 1}, {1, 0}, {0, -1}, {-1, 0} };
	uint8_t mask = 0;
	for (int i = 0; i < 4; i++)
	{
		const Id id = map.tile_id(tile_coords + offsets[i]);
		const auto neighbor_struct = (id ? map.tiles[id_to_index(id)].structure : 0);
		if (tile_struct == neighbor_struct)
		{
			mask |= (1 << i);
		}
	}
	return mask;
}

void Map::update_glyphs(const Box2i dirty_tiles, const Globals& globals)
{
	for (int y = (dirty_tiles.min.y - 1); y <= (dirty_tiles.max.y + 1); y++)
	{
		for (int x = (dirty_tiles.min.x - 1); x <= (dirty_tiles.max.x + 1); x++)
		{
			const Vec2i tile_coords{x, y};
			Id tid = tile_id(tile_coords);
			if (tid)
			{
				const Tile& tile = tiles[id_to_index(tid)];
				TileGlyph& glyph = ground_glyphs[id_to_index(tid)];
				if (tile.structure)
				{
					const auto& struct_type = globals.structure_types[tile.structure];
					glyph.code = struct_type.glyph;
					glyph.color = struct_type.color;
					// TODO: probably more than just wall
					if (struct_type.category == StructureCategory::wall)
					{
						glyph.code += calc_neighbor_matching_structure_mask(*this, tile.structure, tile_coords);
					}
				}
				else
				{
					const auto& terrain_type = globals.terrain_types[tile.terrain];
					glyph.code = terrain_type.glyph;
					glyph.color = terrain_type.color_a;
					if (terrain_type.flags & TileTypeFlags::water)
					{
						const auto mask = calc_neighbor_water_mask(*this, globals.terrain_types, tile_coords);
						if (mask)
						{
							// TODO: maybe provide a function to truncate the code within the page
							glyph.code = (terrain_type.glyph + water_tile_mask.codes[mask]);
						}
					}
				}
			}
		}
	}
}


