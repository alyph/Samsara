
var CARD_WIDTH = 152;
var CARD_HEIGHT = 200;
var CARD_IMG_WIDTH = 144;
var CARD_IMG_HEIGHT = 192;
var TABLE_X = 16;
var PLAYER_HAND_Y = 624;
var PARTY_AREA_Y = 32
var ENTITY_AREA_Y = 512
var CENTER_X = 0;
var CENTER_Y = 472;
var CENTER_HEIGHT = 128;
var SCENE_X = 64;
var SCENE_Y = 96;
var ENCOUNTER_X = 32;
var ENCOUNTER_Y = 64;

var Table = Class(
{
	constructor: function(game)
	{
		this.game = game;
		this.width = PLAYGROUND_WIDTH;
		this.height = PLAYGROUND_HEIGHT;
		this.node = $.playground().addGroup("table", {width: PLAYGROUND_WIDTH, height: PLAYGROUND_HEIGHT});
		this._nextCardId = 0;
		this._playerHand = [];

		this._scene = null;
		this._entity = null;
		this._cards = [];
		this._figures = [];
		this._hand = [];
		this._selectedFigure = null;
		// this._playingDialog = null;
		// this._dialogFinished = null;

		this._background = this.node.addGroup("tableBack", {width : PLAYGROUND_WIDTH, height : PLAYGROUND_HEIGHT});
		this._sceneBack = new Picture(this._background, 0, 0, PLAYGROUND_WIDTH, PLAYGROUND_HEIGHT);

		//this.node.addSprite("tableStage", {animation: Sprites.Overlay_Stage, width: PLAYGROUND_WIDTH, height: PLAYGROUND_HEIGHT});
		//this._stage = $("#tableStage");
		//this._stage.hide();

		//this.center = this.node.addGroup("center", {posx : CENTER_X, posy : CENTER_Y, width : PLAYGROUND_WIDTH, height : CENTER_HEIGHT});
		//this.message = $("<div id='message'></div>").appendTo(this.center);

		//this._encounterArea = new CardArea(this, ENCOUNTER_X, ENCOUNTER_Y, PLAYGROUND_WIDTH, CARD_HEIGHT);
		//Events.bind(this._encounterArea, "CardClicked", this._encounterClicked, this);

		//this._playerHand = new CardArea(this, TABLE_X, PLAYER_HAND_Y, PLAYGROUND_WIDTH, CARD_HEIGHT);
		//this._partyArea = new CardArea(this, TABLE_X, PARTY_AREA_Y, CARD_WIDTH, PLAYGROUND_HEIGHT, { orientation:CardAreaOrientation.Vertical });

		//this._sceneCardPiece = new CardPiece(this, null);
		//this._entityArea = new CardArea(this, TABLE_X, ENTITY_AREA_Y, PLAYGROUND_WIDTH, CARD_HEIGHT, { class: EntityCardPiece });

		//this.figures = this.node.addGroup("figures", {width : PLAYGROUND_WIDTH, height : PLAYGROUND_HEIGHT});

		//this.cardGroup = this.node.addGroup("tableCards", {width : PLAYGROUND_WIDTH, height : PLAYGROUND_HEIGHT});


		
		//this.dialogGroup = this.node.addGroup("tableDialogs", {width : PLAYGROUND_WIDTH, height : PLAYGROUND_HEIGHT});
		this._dialog = new DialogBox(this, "narrativeText");

		this._handArea = new CardAreaBox(this, 32, 608, PLAYGROUND_WIDTH, 192, { horizontalAlignment : HorizontalAlignment.Left, verticalAlignment : VerticalAlignment.Top, padding : 8 });

		this._friendlyArea = new CardAreaBox(this, 0, 352, PLAYGROUND_WIDTH, 1024, { verticalAlignment : VerticalAlignment.Top, padding : 64 });
		this._opposingArea = new CardAreaBox(this, 0, 128, PLAYGROUND_WIDTH, 1024, { verticalAlignment : VerticalAlignment.Bottom, padding : 64 });

		//this.board_location = new LocationBoard(this);

		//this._foreground.click(this._clicked.bind(this));
		//this.node.click(this._clicked.bind(this));
	},

	placeEncounters : function(party)
	{
		this._encounterArea.replaceCards(party.encounters);
	},

	hideEncounters : function()
	{
		this._encounterArea.hide();
	},

	_encounterClicked : function(data)
	{
		this.game.selectEncounter(data.card);
	},

	setupLocation : function(loc)
	{
		this.board_location.setLocation(loc);
		this.board_location.show();
	},

	placeInHand : function(card, slot)
	{
		this._playerHand.placeCard(card, slot);
	},

	placeScene : function(scene)
	{
		if (this._scene !== null)
			throw ("can only place one scene at a time!");

		this._scene = scene;

		/*
		this._scene = scene;
		if (this._entity === null)
			this.setScene(scene, true);*/
	},

	placeStage : function(stage)
	{
		this._stage.fadeIn(500);
	},

	placeEntities : function(entities)
	{
		var num = entities.length;
		var padding = 64;
		var totalWidth = Math.min(num * CARD_WIDTH + (num - 1) * padding, PLAYGROUND_WIDTH - 64);
		var left = Math.floor((PLAYGROUND_WIDTH - totalWidth) / 2);
		var space =  num > 1 ? (totalWidth - CARD_WIDTH) / (num - 1) : 0;
		var top = Math.floor((PLAYGROUND_HEIGHT - CARD_HEIGHT) / 2);
		var rand = 16;

		for (var i = this._cards.length; i < entities.length; i++) 
		{
			this._cards.push(new CardPiece(this));
		};

		for (var i = 0; i < entities.length; i++)
		{
			var card = this._cards[i];
			card.changeCard(entities[i]);
			card.show(left + space * i, MathEx.randomInt(top - rand, top + rand));
		};

		for (var i = entities.length; i < this._cards.length; i++) 
		{
			this._cards[i].hide();
		};

	},

	placeEntity : function(entity, section)
	{
		/*
		var x = MathEx.randomInt(200, 600);
		var y = MathEx.randomInt(400, 600);

		var figure = new Figure(this, entity, x, y);
		this._figures.push(figure);
		Events.bind(figure, 'click', this._figureClicked, this);*/

		this.placeCard(entity, this._friendlyArea);

		/*
		if (section == Sections.Party)
		{
			this._partyArea.placeCard(entity);
		}
		else if (section == Sections.Others)
		{
			this._entityArea.placeCard(entity);	
		}
		else 
		{
			throw ("unkown section:" + section);
		}
		*/
		/*
		if (this._entity !== null)
			this._entity.unselected();

		this._entity = entity;
		this.setScene(entity.scene, false);*/
	},

	placeCard : function(card, area) 
	{
		var piece = new CardPiece(this, card);
		this._cards.push(piece);
		area.placeCard(piece);
	},

	removeEntity : function(entity)
	{
		this._entity = null;
		if (this._scene !== null)
			this.setScene(this._scene, true);
	},

	placeBackground : function(image)
	{
		this._sceneBack.show(image);
	},

	narrate : function(speaker, paragraphs, finished)
	{
		if (speaker !== null)
		{
			var figure = this.findFigure(speaker);
			if (figure)
			{
				figure.text(paragraphs, finished);
				return;
			}
		}

		this._dialog.show(paragraphs, finished);
		// this._playingDialog = this._dialog;
		// this._dialogFinished = finished;

		/*
		this.message.empty();
		for (var i = 0; i < paragraphs.length; i++)
			this.message.append("<p>" + paragraphs[i] + "</p>");*/
	},

	setScene : function(scene, replaceEntities)
	{
		/*
		if (scene.portrait === null)
		{
			this._sceneCardPiece.hide();
		}
		else
		{
			this._sceneCardPiece.changeCard(scene.portrait);
			if (!this._sceneCardPiece.visible)
				this._sceneCardPiece.show(SCENE_X, SCENE_Y)
		}*/

		this._sceneBack.show(scene.background);

		if (replaceEntities)
			this._entityArea.replaceCards(scene.entities);

		this.message.empty();
		for (var i = 0; i < scene.paragraphs.length; i++)
			this.message.append("<p>" + scene.paragraphs[i] + "</p>");

		if (scene.choices.length > 0)
		{
			var choices = $("<ul></ul>").appendTo(this.message);

			for (i = 0; i < scene.choices.length; i++)
			{
				var choice = scene.choices[i];
				var choiceNode = $("<li class='option'>" + choice.text + "</li>").appendTo(choices);
				choiceNode.click({ choice : choice }, this._choiceClicked);
			}
		}
	},

	findFigure : function(name)
	{
		var def = Core.getCard(name);
		var l = this._figures.length;
		for (var i = 0; i < l; i++)
		{
			if (this._figures[i].entity.definition === def)
				return this._figures[i];
		}

		return null;
	},

	findCardPieceByName : function(name)
	{
		var def = Core.getCard(name);
		var l = this._cards.length;
		for (var i = 0; i < l; i++)
		{
			if (this._cards[i].getCard().definition === def)
				return this._cards[i];
		}

		return null;
	},

	_figureClicked : function(data, figure)
	{
		if (this._selectedFigure !== figure)
		{
			this._selectedFigure = figure;
			this._updateHand(figure.entity.hand);
		}
	},

	_updateHand : function(hand)
	{		
		for (var i = 0;  i < this._hand.length; i++)
		{
			if (i < hand.length)
			{
				this._hand[i].changeCard(hand[i]);
			}
			else
			{
				this._hand[i].hide();
			}
		}

		for (var i = this._hand.length; i < hand.length; i++)
		{
			var piece = new CardPiece(this, hand[i]);
			this._cards.push(piece);
			this._hand.push(piece);
		}

		this._handArea.replaceCards(this._hand.slice(0, hand.length));
	},

	_choiceClicked : function(e)
	{
		e.stopPropagation();
		var choice = e.data.choice;
		choice.handler(choice.data);
	},

	_clicked : function(e)
	{
		//if (this._scene !== null && this._scene.isPaused())
		//	this._scene.resume();

		// if (this._playingDialog !== null && this._playingDialog.isWaiting())
		// {
		// 	e.stopImmediatePropagation();
		// 	this._playingDialog.hide(this._dialogFinished);
		// }
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
			optNode.click({ option : option, game : this.game }, function(evt)
			{
				evt.data.option.respond(evt.data.game);
			});
		}
	}
});

var Picture = Class(
{
	$statics:
	{
		nextId : 0
	},

	constructor : function(parent, x, y, width, height)
	{
		this._id = Image.nextId++;
		this._parent = parent;
		this._x = x;
		this._y = y;
		this._width = width;
		this._height = height;
		this._node = null;
		this._sprite = null;
	},

	show : function(sprite)
	{
		if (this._sprite === sprite)
			return;

		this._sprite = sprite;

		if (this._node === null)
		{
			this._parent.addSprite("ImageSprite_" + this._id, {animation: sprite, width: this._width, height: this._height, posx: this._x, posy: this._y});
			this._node = $("#ImageSprite_" + this._id);
		}
		else
		{
			if (sprite !== null)
			{
				this._node.css("background-image", "url("+sprite.imageURL+")");
				this._node.show();
			}
			else
			{
				this._node.hide();
			}
		}
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

