var Card = Class(
{
	constructor : function(game, info, id, slot)
	{
		this._game = game;
		this._table = game.table;
		this.instance = info;
		this.definition = info.definition;
		this.slot = slot;
		this.width = CARD_WIDTH;
		this.height = CARD_HEIGHT;
		this.node = this._table.node.addGroup("card_"+id, {width : this.width, height : this.height});
		var back = $("<div class='cardBack'></div>").appendTo(this.node);
		var sprite = Sprites[this.definition.image];
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

		var label = $("<div class='cardTitle'></div>").html(this.definition.title).appendTo(back);
		var desc = $("<div class='cardDesc'></div>").html(this.definition.desc).appendTo(back);

		this._populateMessage();

		this.node.click( { card : this }, function(evt){ evt.data.card._clicked(); });
	},

	move : function(x, y)
	{
		this.node.xy(x, y, true);
	},

	destroy : function()
	{
		if (this.node != null)
		{
			this.node.remove();
			this.node = null;
		}
	},

	_populateMessage: function()
	{
		this.message = new Message(this.definition.detailed);
		var comps = this.definition.getComponents();
		var actions = [];
		var last = 0;
		for (var i = 0; i < comps.length; i++)
		{
			var comp = comps[i];
			comp.addAction(this._game, this, actions);
			for (; last < actions.length; last++)
			{
				var action = actions[last];
				action.comp = comp;
				this.message.addOption(action.text, action, this, this._performAction);
			}
		}
	},

	_clicked : function()
	{
		this._game.setActiveCard(this);
	},

	_performAction : function(game, action)
	{
		action.handler.call(action.comp, game, this, action.data);
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
	}
});