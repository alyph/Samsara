'use strict';


var FieldArea =
{
	Undefined: 0,
	Party: 1,
	Encounter: 2,
	Environment: 3,
};

/*exported Field*/
class Field extends Entity
{
	constructor()
	{
		super();

		this.cards = [];
	}

	getCardsInArea(area)
	{
		let foundCards = [];
		for (let i = 0; i < this.cards.length; i++) 
		{
			let card = this.cards[i];
			if (card.fieldState.area === area)
				foundCards.push(card);
		}
		return foundCards;
	}
}



