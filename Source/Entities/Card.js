var Card = Class(
{
	constructor : function(game, info, id, slot)
	{
		this._game = game;
		this._table = game.table;
		this.instance = info;
		this.definition = info.definition;
		this._id = id;
		this.slot = slot;
		this.width = CARD_WIDTH;
		this.height = CARD_HEIGHT;
		this.visible = false;
		this.node = null;
		this.activities = [];
		this._populateMessage();
		this._populateActivities();
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
        var sprite = Sprites[this.definition.image];
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

        var label = $("<div class='cardTitle'></div>").html(this.definition.title).appendTo(back);
        var desc = $("<div class='cardDesc'></div>").html(this.definition.desc).appendTo(back);

        this.node.click( { card : this }, function(evt){ evt.data.card._clicked(); });
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

	_populateActivities : function()
	{
		var activities = [];
		var comps = this.definition.getComponents();
		for (var i = 0; i < comps.length; i++)
		{
			var comp = comps[i];
			comp.addActivity(this._game, this, activities);
		}

		this.activities = [];
		for (var i = 0; i < activities.length; i++)
		{
			this.activities.push(Core.getDef(activities[i]));
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
	// TODO: should probably use the base card system now, move these into the ultimate base card
	comps : "",
	title : "[NO TITLE]",
	image : 'cardShortSword',
	desc : "",
	detailed : "[NO DETAILED DESC]",

	constructor : function(def)
	{
		$.extend(this, def);

		this._raw = def;
		var components = this.comps.split(' ');
		this._components = [];
		for (var i = 0; i < components.length; i++)
		{
			var name = components[i];
			if (name === "")
				continue;

			var comp = Core.getComponent(name);
			var superComp = comp.superComponent;

			// TODO: this is wrong, shouldn't add superComp to comps
			// otherwise some function will be called multiple times
			// should probably have another variable just to flag whether
			// a component exists in this card, maybe just use a dictionary
			while (superComp != null)
			{
				this._addComponent(superComp);
				superComp = superComp.superComponent;
			}

			this._addComponent(comp);
		}
	},

	postLoad : function()
	{
		this.eachComp("postLoad", this._raw);
		delete this._raw;
	},

	eachComp : function(func)
	{
		var comps = this._components;
		for (var i = 0; i < comps.length; i++)
		{
			comps[i][func].apply(this, Array.prototype.slice.call(arguments, 1));
		}
	},

	_addComponent : function(comp)
	{
		if (this._components.indexOf(comp) < 0)
		{
			this._components.push(comp);
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

var CardPool = Class(
{
    constructor : function(game)
    {
        this._game = game;
        this._nextCardId = 0;
    },

	makeCard : function(cardInst)
	{
		return new Card(this._game, cardInst, this._nextCardId++, 0);
	}
});