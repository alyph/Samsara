var Deck = Class(
{
	constructor : function(game)
	{
		this._game = game;
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

	_loadCardsWithComp : function(cards, comp)
	{
		for (var i = 0; i < cards.length; i++)
		{
			var card = cards[i];
			if (card.has(comp))
			{
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

	shuffle : function()
	{
		for (i = 0; i < this.cards.length - 1; i++)
		{
			var rand = MathEx.randomInt(i, this.cards.length - 1);
			if (rand != i)
			{
				var tempCard = this.cards[i];
				this.cards[i] = this.cards[rand];
				this.cards[rand] = tempCard;
			}
		}
	},

	draw : function()
	{
		if (this.cards.length === 0)
			throw ("There are no cards in deck!");

		var drawnCard = MathEx.randomElementOfArray(this.cards);
		return this._game.makeCard(drawnCard);
	},

	getCard : function(name)
	{
		var l = this.cards.length;
		for (var i = 0; i < l; i++)
		{
			var ci = this.cards[i];
			if (ci.definition.name === name)
				return this._game.makeCard(ci);
		}

		throw ("no card " + name + " found in deck");
	}
});

var QuestDeck = Class(Deck,
{
	constructor : function(game, cards)
	{
		QuestDeck.$super.call(this, game);

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
	constructor : function(game, cards)
	{
		MapDeck.$super.call(this, game);

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


var EntityDeck = Class(Deck,
{
	constructor :function(game, cards)
	{
		EntityDeck.$super.call(this, game);
		this._loadCardsWithComp(cards, 'Entity');
		this.shuffle();

		this._current = 0;
	},

	drawEntity : function(tags)
	{
		var shuffled = 0;
		while (shuffled <= 1)
		{
			if (this._current >= this.cards.length)
			{
				this._current = 0;
				this.shuffle();
				shuffled++;
			}

			var index = this._current++;
			var card = this.cards[index];

			if (this._matchesTags(card, tags))
			{
				var rand = MathEx.randomInt(0, this.cards.length - 1);
				if (rand !== index)
				{
					this.cards.splice(index, 1);
					this.cards.splice(rand, 0, card);
				}
				return this._game.makeCard(card);
			}
		}
		throw ("cannot find entity that matches tags: " + tags);
	},

	_matchesTags : function(card, tags)
	{
		if (typeof tags === 'string')
		{
			return this._matchesTagString(card, tags);
		}
		else
		{
			for (var i = 0; i < tags.length; i++)
			{
				if (this._matchesTagString(card, tags[i]))
					return true;
			}
			return false;
		}
	},

	_matchesTagString : function(card, tagStr)
	{
		var tags = tagStr.split(',');
		for (var i = 0; i < tags.length; i++)
		{
			var tag = tags[i].trim();
			if (!card.definition.hasTag(tag))
				return false;
		}
		return true;
	}
});

var SiteDeck = Class(Deck,
{
	constructor :function(game, cards)
	{
		SiteDeck.$super.call(this, game);
		this._loadCardsWithComp(cards, 'Site');
	}
});

var CompanyDeck = Class(Deck,
{
	constructor :function(game, cards)
	{
		CompanyDeck.$super.call(this, game);
		this._loadCardsWithComp(cards, 'Company');
	}
});

var EncounterDeck = Class(Deck,
{
	constructor : function(game, cards)
	{
		EncounterDeck.$super.call(this, game);

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
	constructor : function(game)
	{
		PlayerDeck.$super.call(this, game);

		this.drawStartingCards('Follower', 10);
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

var CharacterDeck = Class(Deck,
{
	constructor :function(game, cards)
	{
		CharacterDeck.$super.call(this, game);
		this._loadCardsWithComp(cards, 'Character');
	}
});

var UnitDeck = Class(Deck,
{
	constructor :function(game, cards)
	{
		UnitDeck.$super.call(this, game);
		this._loadCardsWithComp(cards, 'Unit');
	},

	drawFromFaction : function(faction)
	{
		var factionCards = [];

		for (var i = 0; i < this.cards.length; i++)
		{
			var card = this.cards[i];
			if (card.definition.faction == faction)
				factionCards.push(card);
		}

		if (factionCards.length === 0)
			throw ("UnitDeck does not contain cards from faction " + faction);

		var drawnCard = MathEx.randomElementOfArray(factionCards);
		return this._game.makeCard(drawnCard);
	}
});