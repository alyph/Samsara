var StageDefinition = Class(
{
	constructor : function() {}
});

var Stage = Class(
{
	constructor : function(game, party, def, site)
	{
		this._game = game;
		this._party = party;
		this._site = site || null;
		this.def = def;
		this.entities = [];
	},

	populate : function()
	{
		this.entities.push(this._game.companyDeck.draw());
	},

	scouting : function()
	{
		return "Nothing interesting here.";
	},

	detected : function()
	{
		return this.entities;
	}
});