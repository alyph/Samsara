#pragma once

#include "engine/array.h"
#include "engine/random.h"

template<class T>
size_t random_weighted_array_index(const Array<T>& array)
{
	int total_score = 0;
	for (const auto& elem : array)
	{
		asserts(elem.score >= 0);
		total_score += elem.score;
	}
	const auto roll = rand_int(total_score);
	int running = 0;
	size_t rolled_idx = 0;
	for (const auto& elem : array)
	{
		running += elem.score;
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


