var Entity = Class(
{
	constructor : function()
	{
		this.game = null;
		this.world = null;
	},

	beginPlay : function(game, world)
	{
		this.game = game;
		this.world = world;
	},

	getTitle : function()
	{
		return "[NO TITLE]";
	},

	getDesc : function()
	{
		return "[NO DESC]";
	},

	getImage : function()
	{
		return "cardCityGate";
	},

	toString : function()
	{
		if (this.$name !== undefined)
			return this.$name;

		throw ("no valid implementation for entity toString!");
	}
});

