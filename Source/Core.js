var Core =
{
	_hasInit : false,
	_components : {},

	_cardSets : [],
	_cards : {},

	init : function()
	{
		this._hasInit = true;
		this._loadCardSets();
	},

	Component : function(name, cls)
	{
		if (name in this._components)
			throw ("Component" + name + "already registered!");

		var comp = new (Class(cls))();
		comp.name = name;
		this._components[name] = comp;
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

	getCards : function()
	{
		var cards = [];
		for (var name in this._cards)
		{
			if (this._cards.hasOwnProperty(name))
				cards.push(this._cards[name]);
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
		for (var name in set)
		{
			if (set.hasOwnProperty(name))
			{
				if (this._cards.hasOwnProperty(name))
					throw ("multiple cards have same name "+name);

				var def =  new CardDefinition(set[name]);
				def.name = name;
				this._cards[name] = def;
			}
		}
	}
};

var CardComponent = Class(
{
	addOption : function(message) { }
});

