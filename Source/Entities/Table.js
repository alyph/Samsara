var CARD_WIDTH = 106;
var CARD_HEIGHT = 138;
var CARD_IMG_WIDTH = 96;
var CARD_IMG_HEIGHT = 128;
var TABLE_X = 10;
var PLAYER_HAND_Y = 624;
var PARTY_AREA_Y = 400
var ENTITY_AREA_Y = 300
var CENTER_X = 0;
var CENTER_Y = 64;
var CENTER_HEIGHT = 480;
var SCENE_X = 64;
var SCENE_Y = 96;

var Table = Class(
{
	constructor: function(game)
	{
		this._game = game;
		this.node = $.playground().addGroup("table", {width: PLAYGROUND_WIDTH, height: PLAYGROUND_HEIGHT});
		this._nextCardId = 0;
		this._playerHand = [];

		this.center = this.node.addGroup("center", {posx : CENTER_X, posy : CENTER_Y, width : PLAYGROUND_WIDTH, height : CENTER_HEIGHT});
		this.message = $("<div id='message'></div>").appendTo(this.center);

		this._playerHand = new PlayArea(this, TABLE_X, PLAYER_HAND_Y, PLAYGROUND_WIDTH, CARD_HEIGHT);
		this._partyArea = new PlayArea(TABLE_X, PARTY_AREA_Y, PLAYGROUND_WIDTH, CARD_HEIGHT);

		this._sceneCardForm = new CardForm(this, null);
		this._entityArea = new PlayArea(this, TABLE_X, ENTITY_AREA_Y, PLAYGROUND_WIDTH, CARD_HEIGHT);
	},

	placeInHand : function(card, slot)
	{
		this._playerHand.placeCard(card, slot);
	},

	placeScene : function(scene)
	{
		this._sceneCardForm.changeCard(scene.sceneCard);
		if (!this._sceneCardForm.visible)
			this._sceneCardForm.show(SCENE_X, SCENE_Y)

		this._entityArea.replaceCards(scene.entityCards);
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
	}
});

var PlayArea = Class(
{
	constructor : function(table, x, y, w, h)
	{
		this._table = table;
		this.x = x;
		this.y = y;
		this.w = w;
		this.h = h;
		this.cardForms = [];
		this.numSlots = 0;
	},

	placeCard : function(card, slot)
	{
		var i = (arguments.length >= 2 && slot >= 0) ? slot : this.cards.length;
		var x = this.x + i * (CARD_WIDTH + 10);
		var y = this.y;

		if (this.cardForms[i] === null || this.cardForms[i] === undefined)
			this.cardForms[i] = new CardForm(this._table, card);
		else
			this.cardForms[i].changeCard(card);

		this.cardForms[i].show(x, y);
		this.numSlots = Math.max(this.numSlots, i + 1);
	},

	replaceCards : function(cards)
	{
		for (var i = 0; i < cards.length; i++)
		{
			this.placeCard(cards[i], i);
		}

		for (var i = cards.length; i < this.numSlots; i++)
		{
			if (this.cardForms[i])
				this.cardForms[i].hide();
		}

		this.numSlots = cards.length;
	}
});

var CardForm = Class(
{
	$statics:
	{
		nextId : 0
	},

	constructor : function(table, card)
	{
		this._table = table;
		this._id = CardForm.nextId++;
		this.width = CARD_WIDTH;
		this.height = CARD_HEIGHT;
		this.visible = false;
		this.node = null;
		this._card = null;
		this._def = null;

		if (card)
			this.changeCard(card);
	},

	changeCard : function(card)
	{
		if (this._card !== card)
		{
			this._card = card;
			this._def = card.definition;

			if (this.node !== null)
			{
				$("#"+ "cardImage_" + this._id).css("background-image", "url("+Sprites[this._def.image].imageURL+")");
				$("#"+ "cardLabel_" + this._id).html(this._def.title);
				$("#"+ "cardDesc_" + this._id).html(this._def.desc);
			}
		}
	},

	show : function(x, y)
	{
		if (!this.visible)
		{
			if (this.node == null)
				this._initNode();
			else
				this.node.appendTo(this._table.node);

			this.visible = true;
		}

		if (arguments.length >= 2)
			this.moveTo(x, y);
	},

	hide : function()
	{
		if (this.visible)
		{
			if (this.node != null)
				this.node.remove();

			this.visible = false;
		}
	},

	move : function(x, y)
	{
		this.node.xy(x, y, true);
	},

	moveTo : function(x, y)
	{
		this.node.xy(x, y, false);
	},

	destroy : function()
	{
		if (this.node != null)
		{
			this.node.remove();
			this.node = null;
		}
	},

	_initNode : function()
	{
		this.node = this._table.node.addGroup("card_"+this._id, {width : this.width, height : this.height});
		var back = $("<div class='cardBack'></div>").appendTo(this.node);
		var sprite = Sprites[this._def.image];
		var spriteId = "cardImage_" + this._id;
		back.addSprite(spriteId, {animation: sprite, width: CARD_IMG_WIDTH, height: CARD_IMG_HEIGHT, posx:6, posy: 6 });

		var backSize = Math.round(sprite.domO.naturalWidth / sprite.delta) * CARD_IMG_WIDTH;
		var offsetX = -Math.round(sprite.offsetx / sprite.delta) * CARD_IMG_WIDTH;
		var offsetY = -Math.round(sprite.offsety / sprite.distance) * CARD_IMG_HEIGHT;

		$("#"+spriteId).css(
			{
				"background-size" : backSize + "px",
				"background-position" : offsetX + "px " + offsetY + "px",
				"border-radius" : "5px"
			});

		var label = $("<div class='cardTitle'></div>").attr("id","cardLabel_" + this._id).html(this._def.title).appendTo(back);
		var desc = $("<div class='cardDesc'></div>").attr("id","cardDesc_" + this._id).html(this._def.desc).appendTo(back);

		this.node.click( { card : this }, function(evt){ evt.data.card._clicked(); });
	},

	_clicked : function()
	{
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