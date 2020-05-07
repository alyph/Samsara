#include "map.h"

void Map::expand_to_fit_chunk(const IVec2 coords)
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


