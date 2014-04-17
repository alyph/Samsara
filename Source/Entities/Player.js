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
		var screen = this.game.screen;
		screen.background.setData(Sprites[this.party.Locale.background]);
		screen.entities.setData(this.party.getActivities());
	}
});