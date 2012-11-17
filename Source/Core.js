var Core =
{
	_hasInit : false,
	_components : {},
	_componentClasses : {},

	_cardSets : [],
	_cards : {},

	init : function()
	{
		this._hasInit = true;
		this._loadCardSets();
	},

	Component : function(name, base, cls)
	{
		if (name in this._components)
			throw ("Component " + name + " already registered!");

		var superCls = CardComponent;
		var superComp = null;
		if (arguments.length <= 2)
		{
			cls = base;
		}
		else
		{
			superCls = this._componentClasses[base];
			superComp = this._components[base];
			if (superCls === undefined)
				throw ("Component " + name + "'s " + "super class " + base + " cannot be found!");
		}

		var compCls = Class(superCls, cls);
		var comp = new compCls();
		comp.name = name;
		comp.superComponent = superComp;
		this._components[name] = comp;
		this._componentClasses[name] = compCls;
	},

	getComponent : function(name)
	{
		if (!this._components.hasOwnProperty(name))
			throw ("Component " + name + " not found!");

		return this._components[name];
	},

	CardSet : function(set)
	{
		this._cardSets.push(set);
	},

	getCard : function(name)
	{
		if (!this._cards.hasOwnProperty(name))
			throw ("cannot find card definition " + name);

		return this._cards[name];
	},

	getCards : function(comp)
	{
		if (arguments.length <= 0)
			comp = "";

		var cards = [];
		for (var name in this._cards)
		{
			if (this._cards.hasOwnProperty(name))
			{
				var card = this._cards[name];

				if (comp !== "" && !card.has(comp))
					continue;

				cards.push(this._cards[name]);
			}
		}
		return cards;
	},

	_loadCardSets : function()
	{
		for (var i = 0; i < this._cardSets.length; i++)
			this._loadCardSet(this._cardSets[i]);
	},

	_loadCardSet : function(set)
	{
		var base = set.Base;

		for (var name in set)
		{
			if (name === "Base")
				continue;

			if (set.hasOwnProperty(name))
			{
				if (this._cards.hasOwnProperty(name))
					throw ("multiple cards have same name "+name);

				var def = this._extendCardDef(base, set[name]);
				var card =  new CardDefinition(def);
				card.name = name;
				this._cards[name] = card;
			}
		}
	},

	_extendCardDef : function(base, child)
	{
		if (base === undefined)
			return child;

		var def = $.extend({}, base, child);

		if (base.comps && child.comps)
			def.comps = base.comps + " " + child.comps;

		return def;
	}
};

var CardComponent = Class(
{
	constructor : function()
	{
		this.name = "NoName";
		this.superComponent = null;
	},

	addAction : function(game, card, actions) { }
});

