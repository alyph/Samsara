#pragma once

#include "globals.h"

struct Card
{
	uint32_t type;
};

using CardPile = Array<Card>;

extern void shuffle(CardPile& pile);
extern int draw_top_cards(CardPile& src_pile, CardPile& dest_pile, int count);
extern int draw_random_cards(CardPile& src_pile, CardPile& dest_pile, int count);










