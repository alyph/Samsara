var Adventure = Class(
{
	constructor : function(game, slot)
	{
		this._game = game;
		this.slot = slot;
		this.quest = null;
		this.map = null;
		this.encounter = null;
	},

	begin : function()
	{
		this.drawEncounter();
	},

	drawEncounter : function()
	{
		if (this.quest == null || this.map == null)
			throw ("Adventure must have quest and map!");

		var encounterCard = this._game.makeCard(Decks.encounterDeck.drawEncounter());
		this.encounter = new Encounter(this, encounterCard);
		this._game.onNewEncounter(this);
	}
});

var Quest = Class(
{
	constructor : function(game, questCard)
	{
		this._game = game;
		this.questCard = questCard;
	}
});

var Encounter = Class(
{
	constructor : function(adventure, encounterCard)
	{
		this._adventure = adventure;
		this.encounterCard = encounterCard;
	}
});