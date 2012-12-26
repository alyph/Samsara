var Game = Class(
{
	constructor : function()
	{
		this.table = new Table(this);
		this.currentParty = null;

        this._cardPool = new CardPool(this);
	},

	start : function()
	{
		this.initDecks();
		this.player = new Player(this);

		// TODO: draw starting hero and follower for the quest

		this.changeParty(new Party(this));

		// start first turn
		this.beginFirstTurn();
	},

	initDecks : function()
	{
		var cards = Core.getCards();
		this.locationDeck = new LocationDeck(this, cards);
	},

	nextTurn : function()
	{
		this.endTurn();
		this.beginTurn();
	},

	beginFirstTurn : function()
	{
		this.currentParty.drawAgenda(3);
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

	changeParty : function(party)
	{
		this.currentParty = party;
		this.updateScene();
		Events.delegate(this.currentParty, "ActivityChanged", this.updateScene, this);
	},

	updateScene : function()
	{
		this.table.placeScene(this.currentParty.currentActivity.scene);
		Events.delegate(this.currentParty.currentActivity, "SceneChanged", this.updateScene, this);
		Events.delegate(this.currentParty.currentActivity.scene, "SceneUpdated", this.updateScene, this);
	},

	setActiveCard : function(card)
	{
		this.table.placeActiveCard(card);
	},

	makeCard : function(cardInst)
	{
		return this._cardPool.makeCard(cardInst);
	}
});