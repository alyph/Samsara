// Map Cars
Core.CardSet(
{
	City:
	{
		comps : "Map",
		title : "City",
		explorations : [ 'CityStreets' ]
	},

	Forest:
	{
		comps : "Map",
		title : "Forest",
		explorations : [ 'ForestPassage' ]
	},

	Battlefield:
	{
		comps : "Map",
		title : "Battlefield",
		explorations : [ 'Trench' ]
	}
});

// Others
Core.CardSet(
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
		comps : "Explore",
		title : "Streets",
		image : 'cardStreets',
		detailed : "Streets in common area appears dirty and dated, full of buzz and noise."
	},

	ForestPassage:
	{
		comps : "Explore",
		title : "Forest Passage",
		image : 'cardStreets',
		detailed : "A small passage way emerges between the giant sentinel pines, barely visible, leading into deeper darkness."
	},

	Trench:
	{
		comps : "Explore",
		title : "Trench",
		image : 'cardStreets',
		detailed : "Terrain is completely devastated, leaving trenches like withered vines stretching miles and miles long."
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
});