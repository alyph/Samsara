var Entity = Class(
{
	constructor : function()
	{
		this.$ref = true;
		this.game = null;
		this.world = null;
	},

	beginPlay : function(game, world)
	{
		this.game = game;
		this.world = world;
	},

	endPlay : function()
	{
		this.game = null;
		this.world = null;
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

	getSize: function()
	{
		return "";
	},

	toString : function()
	{
		if (this.$name !== undefined)
			return this.$name;

		throw ("no valid implementation for entity toString!");
	}
});

