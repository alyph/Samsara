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
				var card = Cards[name];
				var num = def[name];
				for (var i = 0; i < num; i++)
					this.cards.push(new CardInfo(card));
			}
		}
	},

	draw : function()
	{
		return MathEx.randomElementOfArray(this.cards);
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
	constructor : function(def)
	{
		this.definition = def;
	}
});