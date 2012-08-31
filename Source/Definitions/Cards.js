var CardTypes =
{
	Map : 0,
	Common : 2
};

var MapCards =
{
	City:
	{
		type : CardTypes.Map,
		title : "City",
		explorations : [ 'CityStreets' ]
	},

	Forest:
	{
		type : CardTypes.Map,
		title : "Forest",
		explorations : [ 'ForestPassage' ]
	},

	Battlefield:
	{
		type : CardTypes.Map,
		title : "Battlefield",
		explorations : [ 'Trench' ]
	}
};

var CommonCards =
{
	// city locations
	CityGate:
	{
		title : "City Gate",
		image : 'cardCityGate'
	},

	Tavern:
	{
		title : "Tavern",
		image : 'cardTavern'
	},

	CityStreets:
	{
		title : "Streets",
		image : 'cardStreets'
	},

	ForestPassage:
	{
		title : "Forest Passage",
		image : 'cardStreets'
	},

	Trench:
	{
		title : "Trench",
		image : 'cardStreets'
	},

	// weapons

	ShortSword :
	{
		title : "Short Sword",
		image : 'cardShortSword'
	},

	Dagger :
	{
		title : "Dagger",
		image : 'cardDagger'
	},

	WoodenShield :
	{
		title : "Wooden Shield",
		image : 'cardShortSword'
	},

	ShortBow :
	{
		title : "Short Bow",
		image : 'cardBow'
	},

	Hammer :
	{
		title : "Hammer",
		image : 'cardHammer'
	},

	Crossbow :
	{
		title : "Crossbow",
		image : 'cardCrossbow'
	},

	FireBolt :
	{
		title : "Fire Bolt",
		image : 'cardShortSword'
	},

	Revolver :
	{
		title : "Revolver",
		image : 'cardShortSword'
	}
};


var Cards =
{
	DefaultCard :
	{
		type : CardTypes.Common,
		title : "Empty Card",
		image : 'cardShortSword',
		desc : ""
	},

	_cards : {},

	init : function()
	{
		this._cards = {};
		this._loadSet(CommonCards);
		this._loadSet(MapCards);
	},

	_loadSet : function(set)
	{
		for (var name in set)
		{
			if (set.hasOwnProperty(name))
			{
				if (this._cards.hasOwnProperty(name))
					throw ("multiple cards have same name "+name);

				var card = $.extend({}, this.DefaultCard, set[name]);
				card.name = name;
				this._cards[name] = card;
			}
		}
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
	}
};