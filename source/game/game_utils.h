#pragma once

#include "engine/array.h"
#include "engine/random.h"

class Font;

template<class T>
size_t random_weighted_array_index(const Array<T>& array)
{
	int total_weight = 0;
	for (const auto& elem : array)
	{
		asserts(elem.weight >= 0);
		total_weight += elem.weight;
	}
	const auto roll = rand_int(total_weight);
	// printf("roll: %d, total: %d\n", roll, total_weight);
	int running = 0;
	size_t rolled_idx = 0;
	for (const auto& elem : array)
	{
		running += elem.weight;
		if (roll < running)
		{
			break;
		}
		rolled_idx++;
	}
	asserts(rolled_idx < array.size());
	return rolled_idx;
}

template<class T>
const T& random_weighted_array_element(const Array<T>& array)
{
	return array[random_weighted_array_index(array)];
}

extern Id create_font_glyph_page(uint32_t begin_code, const Font& font);

