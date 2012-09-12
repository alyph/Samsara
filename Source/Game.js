var Game = Class(
{
	constructor : function()
	{
		Decks.init();

		this._sceneDeck = null;
		this.player = null;
		this.table = new Table(this);
		this.encounterSize = 7;
	},

	start : function()
	{
		this.player = new Player(this);

		// put down exploration cards
		var startingMaps = Decks.drawMapDecks(1, true);
		for (var i = 0; i < startingMaps.length; i++)
		{
			var card = this.table.placeInScene(startingMaps[i].exploreCards[0].encounterDeck.drawEncounter());
			this.setActiveCard(card);
		}

		// put down heroes
		var heroCards = this.player.heroDeck.cards;
		for (var i = 0; i < heroCards.length; i++)
		{
			this.table.placeHero(heroCards[i]);
		}

		// start first turn
		this.beginTurn();
	},

	nextTurn : function()
	{
		this.endTurn();
		this.beginTurn();
	},

	beginTurn : function()
	{
		// player draw cards
		for (var i = 0; i < this.player.handSize; i++)
		{
			this.table.placeInHand(this.player.deck.draw());
		}
	},

	endTurn : function()
	{
		this.table.clearPlayerHand();
	},

	setActiveCard : function(card)
	{
		this.table.placeActiveCard(card);
	}

});