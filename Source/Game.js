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

	nextTurn : function()
	{
		this.endTurn();
		this.beginTurn();
	},

	beginTurn : function()
	{
		// player draw cards
		for (var i = 0; i < this.player.HandSize; i++)
		{
			this.table.placeInHand(this.player.Deck.draw());
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

var Table = Class(
{
	constructor: function(game)
	{
		this._game = game;
		this.node = $.playground().addGroup("table", {width: PLAYGROUND_WIDTH, height: PLAYGROUND_HEIGHT});
		this._nextCardId = 0;
		this._sceneCards = [];
		this._playerHand = [];

		this.center = this.node.addGroup("center", {posx : 0, posy : 220, width : PLAYGROUND_WIDTH, height : 320});
		this.message = $("<div id='message'></div>").appendTo(this.center);
	},

	placeInHand : function(info, slot)
	{
		var i = (arguments.length >= 2 && slot >= 0) ? slot : this._playerHand.length;
		var card = this._makeCard(info, i);
		var x = TABLE_X + i * (CARD_WIDTH + 10);
		var y = PLAYER_HAND_Y;
		card.move(x, y);
		this._playerHand[i] = card;
		return card;
	},

	placeInScene : function(info, slot)
	{
		var i = (arguments.length >= 2 && slot >= 0) ? slot : this._sceneCards.length;
		var card = this._makeCard(info, i);
		var x = TABLE_X + i * (CARD_WIDTH + 10);
		var y = SCENE_CARDS_Y;
		card.move(x, y);
		this._sceneCards[i] = card;
		return card;
	},

	placeActiveCard : function(card)
	{
		this.setMessage(card.message);
	},

	clearPlayerHand : function()
	{
		for (var i = 0; i < this._playerHand.length; i++)
		{
			var card = this._playerHand[i];
			if (card !== undefined && card !== null)
				card.destroy();
		}

		this._playerHand.length = 0;
	},

	setMessage : function(message)
	{
		this.message.empty();
		this.message.append("<p>" + message.text + "</p>");

		var options = $("<ul></ul>").appendTo(this.message);

		for (var i = 0; i < message.options.length; i++)
		{
			var option = message.options[i];
			var optNode = $("<li class='option'>" + option.text + "</li>").appendTo(options);
			optNode.click({ option : option, game : this._game }, function(evt)
			{
				evt.data.option.respond(evt.data.game);
			});
		}
	},

	_makeCard : function(info, slot)
	{
		return new Card(this._game, info, this._nextCardId++, slot);
	}
});