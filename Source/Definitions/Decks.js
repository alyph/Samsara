var CommonDecks =
{
	PlayerStartingDeck:
	{
		ShortSword : 5,
		Dagger : 5,
		WoodenShield : 3,
		ShortBow : 3,
		FireBolt : 2,
		Revolver : 2
	},

	City:
	{
		CityGate: 1,
		CityStreets: 1,
		Tavern: 1
	},

	Ruin:
	{

	}
};

var Decks =
{
	init : function()
	{
		var cards = Core.getCards();

		this.mapDecks = [];

		for (var i = 0; i < cards.length; i++)
		{
			var card = cards[i];
			if (card.has('Map'))
			{
				var deck = new MapDeck(card, cards);
				this.mapDecks.push(deck);
			}
		}
	},

	drawMapDecks : function(count)
	{
		var decks = this.mapDecks.slice();
		var results = [];

		for (var i = 0; i < count; i++)
		{
			var picked = MathEx.randomInt(0, decks.length - 1);
			var deck = decks[picked];
			decks.splice(picked, 1);
			results.push(deck);
		}

		return results;
	}
};