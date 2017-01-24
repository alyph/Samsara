'use strict';

/*exported World*/
class World
{
	constructor()
	{
		this.game = null;
		this.entities = {};
		
		this.global = null;
		//this.keeper = null;

		this.field = null;
		this.player = null;
		
		this.updateQueue = [];
		this.isUpdating = false;
	}

	start(game)
	{
		this.game = game;

		for (let inst of Archive.instances)
		{
			this.startInstance(inst);
		}
	}

	spawn(base)
	{
		let inst = Archive.create(base);
		if (inst)
		{
			this.startInstance(inst);
		}
		return inst;
	}

	startInstance(inst)
	{
		let start = inst[World.Symbol.start];
		if (start)
		{
			start.call(inst, this);
		}
	}
}

World.Symbol =
{
	start: Symbol("worldStart"),
};



