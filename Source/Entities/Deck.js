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
					this._addCard(card);
			}
		}
	},

	_addCard : function(cardDef)
	{
		var cardInst = new CardInstance(cardDef);
		this.cards.push(cardInst);
		return cardInst;
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

var QuestDeck = Class(Deck,
{
	constructor : function(cards)
	{
		QuestDeck.$super.call(this);

		this.explorationQuestCard = null;

		for (var i = 0; i < cards.length; i++)
		{
			var card = cards[i];
			if (card.has("Quest"))
			{
				var cardInst = this._addCard(card);

				if (card.has("Exploration"))
					this.explorationQuestCard = cardInst;
			}
		}
	}
});

var MapDeck = Class(Deck,
{
	constructor : function(cards)
	{
		MapDeck.$super.call(this);

		for (var i = 0; i < cards.length; i++)
		{
			var card = cards[i];
			if (card.has('Map'))
			{
				this._addCard(card);
			}
		}
	}
});

var EncounterDeck = Class(Deck,
{
	constructor : function(cards)
	{
		EncounterDeck.$super.call(this);

		for (var i = 0; i < cards.length; i++)
		{
			var card = cards[i];
			if (card.has('Encounter'))
			{
				this.cards.push(new CardInstance(card));
			}
		}
	},

	drawEncounter : function()
	{
		return this.draw();
	}
});

var PlayerDeck = Class(Deck,
{
	constructor : function(startingDeck)
	{
		MapDeck.$super.call(this);

		this.drawStartingCards('Follower', 10)
		this.drawStartingCards('Item', 15);
		this.drawStartingCards('Ability', 20);
	},

	drawStartingCards : function(comp, count)
	{
		var cards = Core.getCards(comp);
		for (var i = 0; i < count; i++)
		{
			var card = MathEx.randomElementOfArray(cards);
			this._addCard(card);
		}
	}
});
