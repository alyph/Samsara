#include "card.h"
#include "game_utils.h"

int draw_random_cards(CardPile& src_pile, CardPile& dest_pile, int count)
{
	const int total = std::min(count, (int)src_pile.size());
	for (int i = 0; i < total; i++)
	{
		const auto roll = rand_int(src_pile.size());
		dest_pile.push_back(src_pile.erase(roll));
	}
	return total;
}









