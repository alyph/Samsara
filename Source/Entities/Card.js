var Card = Class(
{
	constructor : function(game, info, id)
	{
		this._game = game;
		this._table = game.table;
		this._inst = info;
		this._def = info.definition;
		this._width = CARD_WIDTH;
		this._height = CARD_HEIGHT;
		this.node = this._table.node.addGroup("card_"+id, {width : this._width, height : this._height});
		var back = $("<div class='cardBack'></div>").appendTo(this.node);
		var sprite = Sprites[this._def.image];
		var spriteId = "cardImage_" + id;
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

		var label = $("<div class='cardTitle'></div>").html(this._def.title).appendTo(back);
		var desc = $("<div class='cardDesc'></div>").html(this._def.desc).appendTo(back);

		this.message = this._inst.populateMessage();

		var card = this;
		this.node.click(function(){ card._clicked(); });
	},

	move : function(x, y)
	{
		this.node.xy(x, y, true);
	},

	_clicked : function()
	{
		this._game.setActiveCard(this);
	}
});

var CardDefinition = Class(
{
	comps : "",
	title : "[NO TITLE]",
	image : 'cardShortSword',
	desc : "",
	detailed : "[NO DETAILED DESC]",

	constructor : function(def)
	{
		$.extend(this, def);

		var components = this.comps.split(' ');
		this._components = [];
		for (var i = 0; i < components.length; i++)
		{
			var comp = components[i];
			if (comp === "")
				continue;

			this._components[i] = Core.getComponent(comp);
		}
	},

	has : function(name)
	{
		var l = this._components.length;
		for (var i = 0; i < l; i++)
		{
			if (this._components[i].name === name)
				return true;
		}
		return false;
	},

	getComponents : function()
	{
		return this._components;
	}
});

var CardInstance = Class(
{
	constructor : function(def)
	{
		this.definition = def;
	},

	populateMessage: function()
	{
		var message = new Message(this.definition.detailed);
		var comps = this.definition.getComponents();
		for (var i = 0; i < comps.length; i++)
		{
			comps[i].addOption(message);
		}
		return message;
	}
});