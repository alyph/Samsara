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
		inhabitants : [ 'Forest', 'Warg' ]
	}/*,


	Battlefield:
	{
		comps : "Map",
		title : "Battlefield",
		explorations : [ 'Trench' ],
		inhabitants : [ 'Berserk' ]
	}*/
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
	},

	BlackBear:
	{
		comps : "Monster",
		title : "Black Bear",
		image : 'cardBlackBear',
		habitats: [ 'Forest' ],
		density : 4,
		detailed : "You spot a grown black bear wondering around the bank of a stream, poking around looking for food."
	},

	BoarWarrior:
	{
		comps : "Monster",
		title : "Boar Warrior",
		image : 'cardBoarWarrior',
		habitats: [ 'Warg' ],
		density : 4,
		detailed : "A small patrol of boar warriors are foraging the surroundings ahead, anything alive could become their preys."
	},

	GiantSpider:
	{
		comps : "Monster",
		title : "Giant Spider",
		image : 'cardGiantSpider',
		habitats: [ 'Forest' ],
		density : 2,
		detailed : "Through the narrow path, behind the thick branches, it appears a huge black fussy creature with eight legs crawling casually on the pale web."
	},

	CynoPriest:
	{
		comps : "Monster",
		title : "Cyno Priest",
		image : 'cardCynoPriest',
		habitats: [ 'Warg' ],
		density : 1,
		detailed : "You found a small Warg formation camping near the water. They seem to be lead by a Cyno Priest who is singing something faintly."
	},

	FungiHarvester:
	{
		comps : "Monster",
		title : "Fungi Harvester",
		image : 'cardFungiHarvester',
		habitats: [ 'Forest' ],
		density : 2,
		detailed : "At first it appears to be a tranquil forest glade, but soon you notice those giant mushrooms are moving, they are collecting the living creatures they farmed in their cryo-garden!"
	},

	WargBerserk:
	{
		comps : "Monster",
		title : "Warg Berserk",
		image : 'cardWargBerserk',
		habitats: [ 'Warg' ],
		density : 3,
		detailed : "A small company of Warg Berserk are guarding a narrow entrance, no idea what's behind."
	},

	CynoAvenger:
	{
		comps : "Monster",
		title : "Cyno Avenger",
		image : 'cardCynoAvenger',
		habitats: [ 'Warg' ],
		density : "Several Cyro Avengers are on their mission of hunting, likely looking for fresh sacrifices for their three headed god."
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
		image : 'cardForestPath',
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