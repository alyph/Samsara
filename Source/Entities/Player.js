var Player = Class(
{
	constructor : function(game)
	{
		this.game = game;
		this.party = new Party(game);
	},

	beginPlay : function()
	{
		this.beginPlanPhase();
	},

	beginPlanPhase : function()
	{
		this.game.table.placeBackground(Sprites[this.party.Locale.background]);
		this.game.table.placeEntities(this.party.getActivities());
	}
});