'use strict';
/* globals World */

/*exported Entity*/
class Entity
{
	constructor()
	{
		this.world = null;
	}

	[World.Symbol.start](world)
	{
		this.world = world;
		this.start();
	}

	start() {}
}

