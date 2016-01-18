
'use strict';

/*exported Entity*/
class Entity extends BaseObject
{
	constructor()
	{
		super();
		//this.$ref = true;
		
		//this.game = null;
		this.world = null;
		//this.needsUpdate = false;
		this.isUpdating = false;
		this.desc = new Description();
		this.portrait = null;
	}

	init()
	{
		if (this.isInstance())
		{
			this.world = Archive.get(Global.WorldName);
			this.world.onEntityEntered(this);
		}
	}

	// enterWorld : function(world)
	// {
	// 	this.world = world;
	// },

	// leaveWorld : function()
	// {
	// 	this.world = null;
	// },

	beginPlay(game)
	{
	}

	endPlay()
	{
	}

	beginUpdate()
	{
		if (this.isUpdating)
			return;

		this.isUpdating = true;
		this.world.addToUpdate(this);
	}

	update()
	{
		throw ("requested to update, but not implemented.");
	}

	Title()
	{
		return "[NO TITLE]";
	}

	Desc()
	{
		return "[NO DESC]";
	}

	Image()
	{
		return "cardCityGate";
	}

	Portrait()
	{
		return this.portrait;
	}

	Size()
	{
		return "";
	}

	toString()
	{
		return this.name();
	}

	// update : function(entry)
	// {
	// 	throw ("must override!");
	// }
}

