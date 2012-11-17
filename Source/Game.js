var Game = Class(
{
	constructor : function()
	{
		Decks.init();

		this._sceneDeck = null;
		this.player = null;
		this.table = new Table(this);
		this.maxNumAdventures = 7;
		this.maxAdvencturesPerQuest = 3;
        this.adventures = [];

        this._cardPool = new CardPool(this);
	},

	start : function()
	{
		this.player = new Player(this);

		// draw adventures for the starting exploration
		for (var i = 0; i < this.maxAdvencturesPerQuest; i++)
		{
			var quest = new Quest(this, this.makeCard(Decks.questDeck.explorationQuestCard));
			var adventure = new Adventure(this, i);
			adventure.quest = quest;
			adventure.map = this.makeCard(Decks.mapDeck.draw());
			this.adventures.push(adventure);

			adventure.begin();
		}

		// TODO: draw starting hero and follower for the quest

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
			this.table.placeInHand(this.makeCard(this.player.deck.draw()));
		}
	},

	endTurn : function()
	{
		this.table.clearPlayerHand();
	},

	setActiveCard : function(card)
	{
		this.table.placeActiveCard(card);
	},

	onNewEncounter : function(adventure)
	{
		this.table.placeAdventure(adventure);
	},

	makeCard : function(cardInst)
	{
		return this._cardPool.makeCard(cardInst);
	}
});