var CARD_WIDTH = 106;
var CARD_HEIGHT = 138;
var CARD_IMG_WIDTH = 96;
var CARD_IMG_HEIGHT = 128;
var TABLE_X = 10;
var SCENE_CARDS_Y = 60;
var PLAYER_HAND_Y = 560;

var Game = Class(
{
	constructor : function()
	{
		Cards.init();
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
		var startingMaps = Decks.drawMapDecks(3);
		for (var i = 0; i < startingMaps.length; i++)
		{
			this.table.placeInScene(startingMaps[i].exploreCards[0]);
		}

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