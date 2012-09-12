// Map Cars
Core.CardSet(
{
	City:
	{
		comps : "Map City",
		title : "City",
		explorations : [ 'CityStreets' ],
		inhabitants : []
	},

	Forest:
	{
		comps : "Map",
		title : "Forest",
		explorations : [ 'ForestPassage' ],
		inhabitants : [ 'ZephiraCorrupted' ]
	},

	Battlefield:
	{
		comps : "Map",
		title : "Battlefield",
		explorations : [ 'Trench' ],
		inhabitants : [ 'Berserk' ]
	}
});

// Heroes
Core.CardSet(
{
	WarMaiden:
	{
		title : "War Maiden",
		image : 'cardBarbarian'
	},

	Wanderer:
	{
		title : "Wanderer",
		image : 'cardBard'
	},

	Outcast:
	{
		title : "Outcast",
		image : 'cardRogue'
	},

	Acolyte:
	{
		title : "Ess Acolyte",
		image : 'cardSorceress'
	}
});

// Monsters
Core.CardSet(
{
	SoulReaver:
	{
		comps : "Monster",
		title : "Soul Reaver",
		image : 'cardSoulReaver',
		habitats: [ 'Berserk' ],
		density : 2
	},

	BerserkVanguard:
	{
		comps : "Monster",
		title : "Berserk Vanguard",
		image : 'cardBerserkVanguard',
		habitats: [ 'Berserk' ],
		density : 6
	},

	PuppetBorn:
	{
		comps : "Monster",
		title : "Puppet Born",
		image : 'cardPuppetBorn',
		habitats: [ 'ZephiraCorrupted' ],
		density : 2
	},

	GreenCorruption:
	{
		comps : "Monster",
		title : "Green Corruption",
		image : 'cardGreenCorruption',
		habitats: [ 'ZephiraCorrupted' ],
		density : 4
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