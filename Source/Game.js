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
		this.siteDeck = new SiteDeck(this, cards);
		this.companyDeck = new CompanyDeck(this, cards);
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

	getContext : function()
	{
		return this.currentParty.story.context;
	},

	changeParty : function(party)
	{
		this.currentParty = party;
		this._updateStory();
		Events.delegate(this.currentParty, "StoryChanged", this._updateStory, this);
	},

	_updateStory : function()
	{
		this._updateScene();
		Events.delegate(this.currentParty.story.scene, "SceneUpdated", this._updateStory, this);
	},

	_updateScene : function()
	{
		this.table.placeScene(this.currentParty.story.scene);
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