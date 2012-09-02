var Deck = Class(
{
	constructor : function()
	{
		this.cards = [];
	},

	_loadDeck : function(def)
	{
		for (var name in def)
		{
			if (def.hasOwnProperty(name))
			{
				var card = Core.getCard(name);
				var num = def[name];
				for (var i = 0; i < num; i++)
					this.cards.push(new CardInstance(card));
			}
		}
	},

	draw : function()
	{
		return MathEx.randomElementOfArray(this.cards);
	},

	getCard : function(name)
	{
		var l = this.cards.length;
		for (var i = 0; i < l; i++)
		{
			var ci = this.cards[i];
			if (ci.name === name)
				return ci;
		}

		throw ("no card " + name + " found in deck");
	}
});

var MapDeck = Class(Deck,
{
	constructor : function(mapCard, allCards)
	{
		MapDeck.$super.call(this);

		this.mapCard = new CardInstance(mapCard);

		this.exploreCards = [];
		for (var i = 0; i < mapCard.explorations.length; i++)
		{
			var exploreCard = Core.getCard(mapCard.explorations[i]);

			// TODO: maybe we should auto attach explore component in this case!
			if (!exploreCard.has("Explore"))
				throw ("Explore card " + exploreCard + " has no Explore component attached!");

			this.exploreCards.push(new CardInstance(exploreCard));
		}
	}
});

var PlayerDeck = Class(Deck,
{
	constructor : function(startingDeck)
	{
		MapDeck.$super.call(this);

		this._loadDeck(startingDeck);
	}
});
