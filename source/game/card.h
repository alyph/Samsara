#pragma once

#include "globals.h"

using CardId = uint32_t;
using CardTypeId = uint32_t;

struct Card
{
	CardId id{};
	CardTypeId type{};
};

using CardPile = Array<Card>;

extern void shuffle(CardPile& pile);
extern Array<CardId> draw_top_cards(CardPile& src_pile, CardPile& dest_pile, int count);
extern Array<CardId> draw_random_cards(CardPile& src_pile, CardPile& dest_pile, int count);










