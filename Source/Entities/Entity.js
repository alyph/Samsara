/*global Entity: true*/

var Entity = Class(BaseObject,
{
	constructor : function()
	{
		Entity.$super.call(this);
		//this.$ref = true;
		
		//this.game = null;
		this.world = null;
		//this.needsUpdate = false;
		this.isUpdating = false;
		this.desc = new Description();
		this.portrait = null;
	},

	init : function()
	{
		if (this.isInstance())
		{
			this.world = Archive.get(Global.WorldName);
			this.world.onEntityEntered(this);
		}
	},

	// enterWorld : function(world)
	// {
	// 	this.world = world;
	// },

	// leaveWorld : function()
	// {
	// 	this.world = null;
	// },

	beginPlay : function(game)
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
		throw ("requested to update, but not implemented.");
	},

	Title : function()
	{
		return "[NO TITLE]";
	},

	Desc : function()
	{
		return "[NO DESC]";
	},

	Image : function()
	{
		return "cardCityGate";
	},

	Portrait : function()
	{
		return this.portrait;
	},

	Size: function()
	{
		return "";
	},

	toString : function()
	{
		return this.name();
	},

	// update : function(entry)
	// {
	// 	throw ("must override!");
	// }
});

