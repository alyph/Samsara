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
		if (this.cards.length === 0)
			throw ("There are no cards in deck!");

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

			var exploreInst = new CardInstance(exploreCard);
			exploreInst.encounterDeck = new EncounterDeck(mapCard, exploreCard, allCards);
			this.exploreCards.push(exploreInst);
		}
	}
});

var EncounterDeck = Class(Deck,
{
	constructor : function(mapCard, exploreCard, allCards)
	{
		EncounterDeck.$super.call(this);

		var inhabitants = mapCard.inhabitants.concat(exploreCard.inhabitants);

		for (var i = 0; i < allCards.length; i++)
		{
			var card = allCards[i];
			if (card.has('Monster') && this._canInhabit(card, inhabitants))
			{
				for (var c = 0; c < card.density; c++)
				{
					this.cards.push(new CardInstance(card));
				}
			}
		}
	},

	_canInhabit : function(monster, inhabitants)
	{
		if (inhabitants.length === 0)
			return false;

		for (var i = 0; i < monster.habitats.length; i++)
		{
			if (inhabitants.indexOf(monster.habitats[0]) >= 0)
			{
				return true;
			}
		}
		return false;
	},

	drawEncounter : function()
	{
		return this.draw();
	}
})

var PlayerDeck = Class(Deck,
{
	constructor : function(startingDeck)
	{
		MapDeck.$super.call(this);

		this._loadDeck(startingDeck);
	}
});
