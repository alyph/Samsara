var Entity = Class(BaseObject,
{
	constructor : function()
	{
		Entity.$super.call(this);
		//this.$ref = true;
		this.isInstance = false;
		//this.game = null;
		this.world = null;
		//this.needsUpdate = false;
		this.isUpdating = false;
		this.desc = new Description();
	},

	enterWorld : function(world)
	{
		this.world = world;
	},

	leaveWorld : function()
	{
		this.world = null;
	},

	beginPlay : function()
	{
	},

	endPlay : function()
	{
	},

	beginUpdate: function()
	{
		if (this.isUpdating)
			return;

		this.isUpdating = true;
		this.world.addToUpdate(this);
	},

	update : function()
	{
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

	getPortrait : function()
	{
		throw ("must override!");
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
	},

	update : function(entry)
	{
		throw ("must override!");
	}
});

