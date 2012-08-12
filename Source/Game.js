var CARD_WIDTH = 100;
var CARD_HEIGHT = 160;
var TABLE_X = 10;
var SCENE_CARDS_Y = 60;
var PLAYER_HAND_Y = 420;

var Game = Class(
{
	constructor : function()
	{
		this._sceneDeck = null;
		this.player = null;
		this.table = new Table(this);
		this.encounterSize = 7;
	},

	start : function()
	{
		this._sceneDeck = new SceneDeck(Decks.Ruin);
		this.player = new Player(this);

		// draw entrance card
		this.table.placeInScene(this._sceneDeck.getEntranceCard());

		// put down exploration card
		for (var i = 1; i < this.encounterSize; i++)
		{
			this.table.placeInScene(this._sceneDeck.getExplorationCard());
		}

		// starting heroes

		// start first turn
		this.beginTurn();
	},

	beginTurn : function()
	{
		// player draw cards
		for (var i = 0; i < this.player.HandSize; i++)
		{
			this.table.placeInHand(this.player.Deck.draw());
		}
	}
});

var Table = Class(
{
	constructor: function(game)
	{
		this._game = game;
		this.Node = $.playground().addGroup("table", {width: PLAYGROUND_WIDTH, height: PLAYGROUND_HEIGHT});
		this._nextCardId = 0;
		this._sceneCards = [];
		this._playerHand = [];
	},

	placeInHand : function(info)
	{
		var card = this._makeCard(info);
		var x = TABLE_X + this._playerHand.length * (CARD_WIDTH + 10);
		var y = PLAYER_HAND_Y;
		card.move(x, y);
		this._playerHand.push(card);
	},

	placeInScene : function(info)
	{
		var card = this._makeCard(info);
		var x = TABLE_X + this._sceneCards.length * (CARD_WIDTH + 10);
		var y = SCENE_CARDS_Y;
		card.move(x, y);
		this._sceneCards.push(card);
	},

	_makeCard : function(info)
	{
		return new Card(this._game, info, this._nextCardId++);
	}
});