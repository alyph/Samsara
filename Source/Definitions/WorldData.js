Global.WorldName = "world";

$inst(Global.WorldName, 
{
	$base: World,
	global: "@global",
	keeper: "@keeper"
});

$begin("locale");
$use("keyword");

$inst("plain_a",
{
	$base: Locale,	
	displayName: "Redgrass March", portrait: "loc_barren",
	desc:$v(
	[
		"bordering @forest_a north",
		"bordering @plain_b south",
		"bordering @hills_a west"
	])
});

$inst("forest_a",
{
	$base: Locale,	
	displayName: "Golden Grove", portrait: "loc_night_forest",
	desc:$v(
	[
		"bordering @plain_a south"
	])
});

$inst("plain_b",
{
	$base: Locale,	
	displayName: "Fen Of Serpents", portrait: "loc_desert_rock",
	desc:$v(
	[
		"bordering @plain_a north",
		"bordering @hills_a west",
		"bordering @forest_b east"
	])
});

$inst("forest_b",
{
	$base: Locale,	
	displayName: "Crown Woods", portrait: "loc_forest",
	desc:$v(
	[
		"bordering @plain_b west"
	])
});

$inst("hills_a",
{
	$base: Locale,	
	displayName: "Hills Of Krondor", portrait: "loc_rocky",
	desc:$v(
	[
		"bordering @plain_b east",
		"bordering @plain_a northeast"
	])
});

$end();

$begin("temp_locale");
$use("keyword");
$use("locale");

$inst("starting_loc",
{
	$base: Locale,	
	displayName: "",
	desc:$v(
	[
		"part_of @plain_a"
	])
});

$end();

$begin("character");

$inst("pc_melee",
{
	$base: Character,
	displayName: "Ragnar",
	sprite: "Cha_Warlord",
	//position: "front"
});

$inst("pc_caster",
{
	$base: Character,
	displayName: "Tiamat",
	sprite: "Cha_Acolyte",
	//position: "rear"
});

$inst("pc_agile",
{
	$base: Character,
	displayName: "Yagnas",
	sprite: "Cha_Deva",
	//position: "front"
});

$end();

$begin("party");
$use("character");

$inst("player_party", 
{
	$base: Party,
	members: [ "@pc_melee", "@pc_caster", "@pc_agile" ]
});

$end();

$begin("pov");
$use("character");
$use("temp_locale");

$inst("player_party_pov", 
{
	$base: "proto.pov_base",
	scene: { $base: "scene.caravan_ambush" },
	locale: "@starting_loc",
	controller: "@player",
	characters: [ "@pc_melee", "@pc_caster", "@pc_agile" ]
});

$end();


$inst("player",
{
	$base: Player
});

$inst("keeper",
{
	$base: Keeper
});

	/*

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
	}
};
*/


