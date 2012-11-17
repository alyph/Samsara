// Map Cars
Core.CardSet(
{
	Zion:
	{
		comps : "Map",
		title : "City of Zion"
	},

	TiberusRiverSide:
	{
		comps : "Map",
		title : "Tiberus River Side"
	},

	MistForest:
	{
		comps : "Map",
		title : "Mist Forest"
	},

	SolomonsFooting:
	{
		comps : "Map",
		title : "Solomon's Footing"
	},

	StonefellMarch:
	{
		comps : "Map",
		title : "Stonefell March"
	},

	ValeOfSerpents:
	{
		comps : "Map",
		title : "Vale of Serpents"
	},

	GodsHighSeat:
	{
		comps : "Map",
		title : "Gods High Seat"
	},

	Silverwood:
	{
		comps : "Map",
		title : "Silverwood"
	},

	SwansRest:
	{
		comps : "Map",
		title : "Swan's Rest"
	},

	SunstoneRidges:
	{
		comps : "Map",
		title : "Sunstone Ridges"
	}

/*
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
	},


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

var BaseFollowerCard =
{
	comps : "Follower"
};

// Followers
Core.CardSet(
{
	Base : BaseFollowerCard,

	WardenArc:
	{
		title : "Warden of Arc",
		image : 'SwordsmenCaptain'
	},

	MoonGladeWhisper:
	{
		title : "Moon Glade Whisper",
		image : 'Druid'
	},

	ThylonWorshiper:
	{
		title : "Thylon Worshiper",
		image : 'BattleCleric'
	},

	Skald:
	{
		title : "Skald",
		image : 'Skald'
	},

	WindSpireApprentice:
	{
		title : "Wind Spire Apprentice",
		image : 'ApprenticeMage'
	},

	MoonGladeRanger:
	{
		title : "Moon Glade Ranger",
		image : 'Ranger'
	},

	MystAgent:
	{
		title : "Myst Agent",
		image : 'Assassin'
	},

	FlameSentinel:
	{
		title : "Flame Sentinel",
		image : 'TempleAcolyte'
	},

	ThracianMilitia:
	{
		title : "Thracian Militia",
		image : 'ElfGuardsman'
	},

	StarSpireAcolyte:
	{
		title : "Star Spire Acolyte",
		image : 'FemaleApprenticeMage'
	},

	QuietIsleWanderer:
	{
		title : "Quiet Isle Wanderer",
		image : 'FemaleWanderer'
	},

	WarMaiden:
	{
		title : "War Maiden",
		image : 'BattleMaiden'
	},

	SkySinger:
	{
		title : "Sky Singer",
		image : 'FemaleBard'
	},

	ThracianScout:
	{
		title : "Thracian Scout",
		image : 'Huntress'
	},

	ConclaveObserver:
	{
		title : "Conclave Observer",
		image : 'Doctor'
	},

	SorrowsHunter:
	{
		title : "Sorrows Hunter",
		image : 'Hunter'
	},

	StepStoneCourier:
	{
		title : "StepStone Courier",
		image : 'Pirate'
	},

	Gunslinger:
	{
		title : "Gunslinger",
		image : 'Gunslinger'
	}
});


Core.CardSet(
{
	Exploration:
	{
		title : "Exploration",
		comps : "Exploration"
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
		density : 2,
		detailed : "Several Cyro Avengers are on their mission of hunting, likely looking for fresh sacrifices for their three headed god."
	}
});

var BaseItemCard =
{
	comps : "Item"
};

// Items
Core.CardSet(
{
	Base : BaseItemCard,

	FineBlade :
	{
		title : "Fine Blade",
		image : 'ShortSword'
	},

	PoisonDagger :
	{
		title : "Poison Dagger",
		image : 'PoisonDagger'
	},

	Spear :
	{
		title : "Spear",
		image : 'Spear'
	},

	Buckler :
	{
		title : "Buckler",
		image : 'Buckler'
	},

	ShortBow :
	{
		title : "Short Bow",
		image : 'ShortBow'
	},

	WarHammer :
	{
		title : "WarHammer",
		image : 'WarHammer'
	},

	LightCrossbow :
	{
		title : "LightCrossbow",
		image : 'LightCrossbow'
	},

	RodIce :
	{
		title : "Rod of Ice Shards",
		image : 'RodOfMissiles'
	},

	FireStaff :
	{
		title : "Fire Staff",
		image : 'FireStaff'
	},

	Pistol :
	{
		title : "Pistol",
		image : 'Pistol'
	},

	PaddedArmor :
	{
		title : "Padded Armor",
		image : 'PaddedVest'
	},

	ChainMail :
	{
		title : "Chain Mail",
		image : 'ChainMail'
	},

	GoldCoins :
	{
		title : "Gold Coins",
		image : 'HandfulGold'
	}
});


BaseAbilityCard =
{
	comps : "Ability"
};

// Abilities
Core.CardSet(
{
	Base : BaseAbilityCard,

	Parry:
	{
		title : "Parry",
		image : 'Parry'
	},

	BackStab:
	{
		title : "Back Stab",
		image : 'BackStab'
	},

	MagicShield:
	{
		title : "Magic Shield",
		image : 'MagicShield'
	},

	BackStab:
	{
		title : "Back Stab",
		image : 'BackStab'
	},

	PiercingBlow:
	{
		title : "Piercing Blow",
		image : 'ArmorPiercing'
	},

	PiercingShot:
	{
		title : "Piercing Shot",
		image : 'PiercingShot'
	},

	CripplingBlow:
	{
		title : "Crippling Blow",
		image : 'CripplingBlow'
	},

	RainOfFire:
	{
		title : "Rain of Fire",
		image : 'RainOfFire'
	},

	LightningStrike:
	{
		title : "Lightning Strike",
		image : 'LightningStrike'
	},

	HealWound:
	{
		title : "Heal Wound",
		image : 'Healing'
	},

	Disarm:
	{
		title : "Disarm",
		image : 'Disarm'
	},

	AimedShot:
	{
		title : "Aimed Shot",
		image : 'AimedShot'
	},

	PointBlankShot:
	{
		title : "Point Blank Shot",
		image : 'PointBlankShot'
	},

	ChargeAttack:
	{
		title : "Charge Attack",
		image : 'Charge'
	},

	PoisonWeapon:
	{
		title : "Poison Weapon",
		image : 'PoisonWeapon'
	},

	RainOfArrows:
	{
		title : "Rain of Arrows",
		image : 'RainOfArrows'
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
	}
});