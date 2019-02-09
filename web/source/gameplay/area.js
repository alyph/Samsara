'use strict';

/* exported Area */
class Area
{
	constructor()
	{
		this.cards = [];
	}

	isEmpty()
	{
		return this.cards.length === 0;
	}

	placeCards(cards)
	{
		for (let card of cards)
		{
			card.placeIn(this);
		}
	}
}

