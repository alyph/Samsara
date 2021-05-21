#include "card.h"
#include "game_utils.h"

Array<CardId> draw_random_cards(CardPile& src_pile, CardPile& dest_pile, int count)
{
	const int total = std::min(count, (int)src_pile.size());
	auto drawn_cards = make_temp_array<CardId>(total);
	for (int i = 0; i < total; i++)
	{
		const auto roll = rand_int(src_pile.size());
		const auto card = src_pile.erase(roll);
		drawn_cards[i] = card.id;
		dest_pile.push_back(card);
	}
	return drawn_cards;
}









