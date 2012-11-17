var CommonDecks =
{
	PlayerStartingDeck:
	{
		ShortSword : 5,
		Dagger : 5,
		WoodenShield : 3,
		ShortBow : 3,
		FireBolt : 2,
		Revolver : 2
	},

	PlayerStartingHeroes:
	{
		WarMaiden : 1,
		Wanderer : 1,
		Outcast : 1,
		Acolyte : 1
	},

	City:
	{
		CityGate: 1,
		CityStreets: 1,
		Tavern: 1
	},

	Ruin:
	{

	}
};

var Decks =
{
	init : function()
	{
		var cards = Core.getCards();

		this.questDeck = new QuestDeck(cards);
		this.mapDeck = new MapDeck(cards);
		this.encounterDeck = new EncounterDeck(cards);
	}
};