World.WorldData.entities =
{
	"Locale.PlainStarFall":
	{
		$base : "Locales.Wildness"
	},

	"Character.Ragnar":
	{
		$base: Character,
		name: "Ragnar",
		sprite: "Cha_Warlord",
		position: "front"
	},

	"Character.Tiamat":
	{
		$base: Character,
		name: "Tiamat",
		sprite: "Cha_Acolyte",
		position: "rear"
	},

	"Character.Yagnas":
	{
		$base: Character,
		name: "Yagnas",
		sprite: "Cha_Deva",
		position: "front"
	}

	/*
	"Locale.OlegTradingPost":
	{
		$base : "Locales.PointOfIntrest",
		background : "Loc_Outpost",
		within : "$Locale.SouthRostland"
	},

	"Locale.SouthRostland":
	{
		$base : "Locales.Wildness",
		connections :
		[
			{ Locale : "$Locale.Kamelands" },
			{ Locale : "$Locale.Narlmarches" }
		]
	},

	"Locale.Kamelands":
	{
		$base : "Locales.Wildness",
		connections :
		[
			{ Locale : "$Locale.SouthRostland" },
			{ Locale : "$Locale.Narlmarches" }
		]
	},

	"Locale.Narlmarches":
	{
		$base : "Locales.Wildness",
		connections :
		[
			{ Locale : "$Locale.Kamelands" },
			{ Locale : "$Locale.SouthRostland" }
		]
	},

	"Region.Greenbelt":
	{
		$base : "Regions.Base",
		areas :
		[
			"$Locale.SouthRostland",
			"$Locale.Kamelands",
			"$Locale.Narlmarches"
		]
	}*/
};
