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
		this._initFromTemplates();
		this._populateActivities();
    },

	beginPlay : function()
	{
		this.eachComp('beginPlay', this._game);
	},

	clicked : function()
	{
		this.eachComp('clicked', this._game);
	},

	_initFromTemplates : function()
	{
		var comps = this.definition.getComponents();
		for (var i = 0; i < comps.length; i++)
		{
			var template = comps[i]._template;
			if (template !== undefined && template !== null)
			{
				for (name in template)
				{
					if (this.hasOwnProperty(name))
						throw ("Conflicting template property: " + name + ", new: " + template[name] + ", existing: " + this[name]);
					this[name] = template[name];
				}
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

	eachComp : function(func)
	{
		var comps = this.definition.getComponents();
		var args = Array.prototype.slice.call(arguments, 1);
		args.unshift(this);
		for (var i = 0; i < comps.length; i++)
		{
			comps[i][func].apply(comps[i], args);
		}
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
	tags : "",

	constructor : function(def)
	{
		$.extend(this, def);

		this._raw = def;
		var components = this.comps.split(' ');
		this._components = [];
		this._hasComps = {};
		for (var i = 0; i < components.length; i++)
		{
			var name = components[i];
			if (name === "")
				continue;

			var comp = Components.get(name);
			this._addComponent(comp);
		}

		var tags = this.tags.split(',');
		this._tags = {};
		for (i = 0; i < tags.length; i++)
		{
			this._tags[tags[i].trim()] = true;
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
			this._hasComps[comp.name] = 1;

			var superComp = comp.base;
			while (superComp !== null)
			{
				this._hasComps[superComp.name] = 1;
				superComp = superComp.base;
			}
		}
	},

	has : function(name)
	{
		return this._hasComps[name];
	},

	getComponents : function()
	{
		return this._components;
	},

	hasTag : function(tag)
	{
		return this._tags[tag] !== undefined;
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

Component('Card',
{
	constructor : function()
	{
		this.name = "NoName";
		this.superComponent = null;
	},

	postLoad : function(def) {},

	beginPlay : function(card, game) {},

	clicked : function(card, game) {},

	addAction : function(game, card, actions) {},

	addActivity : function(game, card, activites) {}
});

Component('Item', 'Card',
{

});

Component('Ability', 'Card',
{

});

Component('Entity', 'Card',
{
	beginPlay : function(card, game)
	{
		card.hand = [];
		var defHand = card.definition.hand;
		var l = defHand.length;
		for (var i = 0; i < l; i++)
		{
			var def = Core.getCard(defHand[i]);
			card.hand.push(game.makeCard(new CardInstance(def)));
		}
	}
});

Component('Character', 'Card',
{

});

Component('Map', 'Card',
{

});
