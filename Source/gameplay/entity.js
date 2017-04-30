'use strict';
/* globals World */

/*exported Entity*/
class Entity
{
	constructor()
	{
		this.world = null;
		this.rule = null;
	}

	[World.Symbol.start](world)
	{
		this.world = world;
		this.rule = world.rule;
		this.start();
	}

	start() {}
}

