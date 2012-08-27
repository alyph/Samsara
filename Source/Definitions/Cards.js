var Cards =
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
	},

	DefaultCard :
	{
		title : "Empty Card",
		image : 'cardShortSword',
		desc : ""
	},

	getCardDefinition : function(name)
	{
		var def = this[name];
		if (def === undefined)
			throw ("cannot find card definition " + name);

		if (def._populated)
			return def;

		def = this[name] = $.extend({}, this.DefaultCard, def);
		def._populated = true;
		return def;
	}
}