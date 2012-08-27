var Deck = Class(
{
	constructor : function(def)
	{
		this.definition = def;
		this.cards = [];

		for (var name in def)
		{
			if (def.hasOwnProperty(name))
			{
				var card = Cards.getCardDefinition(name);
				var num = def[name];
				for (var i = 0; i < num; i++)
					this.cards.push(new CardInfo(name, card));
			}
		}
	},

	draw : function()
	{
		return MathEx.randomElementOfArray(this.cards);
	},

	getCard : function(name)
	{
		var l = this.cards.length;
		for (var i = 0; i < l; i++)
		{
			var ci = this.cards[i];
			if (ci.name === name)
				return ci;
		}

		throw ("no card " + name + " found in deck");
	}
});

var SceneDeck = Class(Deck,
{
	constructor : function(def)
	{
		SceneDeck.$super.call(this, def);
	},

	getEntranceCard : function()
	{
		return new CardInfo({});
	},

	getExplorationCard : function()
	{
		return new CardInfo({});
	}
});

var CardInfo = Class(
{
	constructor : function(name, def)
	{
		this.name = name;
		this.definition = def;
	}
});