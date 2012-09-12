var CARD_WIDTH = 106;
var CARD_HEIGHT = 138;
var CARD_IMG_WIDTH = 96;
var CARD_IMG_HEIGHT = 128;
var TABLE_X = 10;
var SCENE_CARDS_Y = 20;
var PLAYER_HAND_Y = 624;
var HERO_ZONE_Y = 480;

var Table = Class(
{
	constructor: function(game)
	{
		this._game = game;
		this.node = $.playground().addGroup("table", {width: PLAYGROUND_WIDTH, height: PLAYGROUND_HEIGHT});
		this._nextCardId = 0;
		this._sceneCards = [];
		this._playerHand = [];
		this._heroes = [];

		this.center = this.node.addGroup("center", {posx : 0, posy : 172, width : PLAYGROUND_WIDTH, height : 320});
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

	placeHero : function(inst)
	{
		var x = 0;
		var y = HERO_ZONE_Y;
		var newIdx = this._heroes.length;

		for (var i = 0; i < this._heroes.length; i++)
			x += this._heroes[i].width + 10;

		var card = this._makeCard(inst, newIdx);
		card.move(x, y)
		var zone = new HeroZone(this, card, x, y);
		this._heroes[newIdx] = zone;
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

var HeroZone = Class(
{
	constructor : function(table, card, x, y)
	{
		this._game = table.game;
		this._table = table;
		this.width = card.width + 40;
		this.height = card.height + 20;

		this.node = this._table.node.addGroup("heroZone_"+card.id, {posx : x, posy : y, width : this.width, height : this.height});
	}
});